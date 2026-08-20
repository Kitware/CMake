# syntax=docker/dockerfile:1

# Base image for the CMake development container.
#
# The images our CI infrastructure uses, described under `.gitlab/ci/docker/`,
# are built on the distributions we test CMake against and are minimized for
# CI use.  Prepare the environment from scratch on Ubuntu instead: it offers
# the broadest ecosystem of packages and tooling for development, including a
# recent `cmake` and the `clang-format` version our style rules require.
ARG BASE_IMAGE=ubuntu:26.04

FROM ${BASE_IMAGE}

ARG USERNAME=cmake-dev
ARG USER_UID=1000
ARG USER_GID=${USER_UID}

# Install the packages needed to build CMake, run its test suite, build its
# documentation, and satisfy its style rules, along with a few more that make
# the container a comfortable place to work.
#
# Cache the package lists and the downloaded archives, and hide the
# `docker-clean` configuration the base image provides so that it does not
# discard them, so that rebuilding the container downloads only what has
# changed since the last build.
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

# Run the optional local customization scripts.  They are ignored by Git so
# that developers may customize the container without modifying tracked files
# or risking that the customizations end up in a commit.  See
# `Help/dev/devcontainer.rst`.
#
# `USER` sets neither the working directory nor `HOME`, and BuildKit passes a
# `RUN` only the environment the image records, which names just `PATH`.  Say
# where each hook runs and whose home it writes to, so that a hook may spell a
# path relative to either.
WORKDIR /root
RUN --mount=type=bind,source=hooks,target=/opt/cmake-dev-hooks \
    if test -f /opt/cmake-dev-hooks/root.sh; then \
        HOME=/root sh -e /opt/cmake-dev-hooks/root.sh; \
    fi

USER ${USERNAME}
WORKDIR /home/${USERNAME}
RUN --mount=type=bind,source=hooks,target=/opt/cmake-dev-hooks \
    if test -f /opt/cmake-dev-hooks/user.sh; then \
        HOME=/home/${USERNAME} sh -e /opt/cmake-dev-hooks/user.sh; \
    fi
