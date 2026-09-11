# syntax=docker/dockerfile:1

# Base image for the CMake development container.
#
# The images our CI infrastructure uses, described under `.gitlab/ci/docker/`,
# are built on the distributions we test CMake against and are minimized for
# CI use.  Prepare the environment from scratch on Ubuntu instead: it offers
# the broadest ecosystem of packages and tooling for development, including
# the `clang-format` version our style rules require and the Kitware APT
# repository through which CMake itself is published.
ARG BASE_IMAGE=ubuntu:26.04

FROM ${BASE_IMAGE}

ARG USERNAME=cmake-dev
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

# The SHA-256 of the signing key `apt.kitware.com` publishes.  Kitware rotates
# that key every few years; updating this hash both approves the new key and
# forces the step below to fetch it again rather than reuse a cached layer.
# Read the current value with:
# curl -fsSL https://apt.kitware.com/keys/kitware-archive-latest.asc | sha256sum
ARG KITWARE_PUBLIC_KEY_SHA256=801bc629e356c3c96f184351272914222ce427777400fa7d1baed3ab180b3e3b

# Add the Kitware APT repository, which carries CMake releases newer than the
# ones the distribution provides, before installing anything from it below.
RUN --mount=type=bind,source=install_kitware_archive.sh,target=/root/install_kitware_archive.sh \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    sh /root/install_kitware_archive.sh ${KITWARE_PUBLIC_KEY_SHA256}

# Install the packages needed to build CMake, run its test suite, build its
# documentation, and satisfy its style rules, along with a few more that make
# the container a comfortable place to work.
#
# Cache the package lists and the downloaded archives, and hide the
# `docker-clean` configuration the base image provides so that it does not
# discard them.  A rebuild then downloads only what has changed since the
# last build.
RUN --mount=type=bind,source=install_deps.sh,target=/root/install_deps.sh \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=dev_packages.lst,target=/root/dev_packages.lst \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    sh /root/install_deps.sh

# Install the GitLab CLI, `glab`, and `glab-axi`, an agent-ergonomic wrapper
# around it, for interacting with our GitLab instance.  Keep the packages
# `npm` downloads in a cache, and unpack the `glab` release in a `tmpfs`, so
# that neither leaves anything behind in the image.
RUN --mount=type=bind,source=install_glab.sh,target=/root/install_glab.sh \
    --mount=type=cache,target=/root/.npm,sharing=locked \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_glab.sh

# Create an unprivileged user matching the typical host account so that files
# created in the mounted source tree are not owned by `root`.
RUN --mount=type=bind,source=create_user.sh,target=/root/create_user.sh \
    sh /root/create_user.sh ${USERNAME} ${USER_UID} ${USER_GID}

# Run the optional local customization hook for the build, if the developer
# has written one.  It is ignored by Git so that developers may customize the
# container without modifying tracked files or risking that the customizations
# end up in a commit.  See `Help/dev/devcontainer.rst`.
#
# It runs as the container user, who may `sudo`, rather than as `root`: one
# hook that can reach either is simpler to write against than two that each
# reach one.  Mount it beside the dispatcher that runs it, under the same
# parent it has in the source tree, so that the dispatcher locates it here the
# same way it does when a container lifecycle command runs it.
#
# `USER` sets neither the working directory nor `HOME`, and BuildKit passes a
# `RUN` only the environment the image records, which names just `PATH`.  Say
# where the hook runs and whose home it writes to, so that it may spell a path
# relative to either.
USER ${USERNAME}
WORKDIR /home/${USERNAME}
RUN --mount=type=bind,source=run-hooks.sh,target=/opt/cmake-dev/run-hooks.sh \
    --mount=type=bind,source=hooks,target=/opt/cmake-dev/hooks \
    HOME=/home/${USERNAME} sh /opt/cmake-dev/run-hooks.sh build
