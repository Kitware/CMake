# syntax=docker/dockerfile:1

ARG BASE_IMAGE=debian:12

FROM ${BASE_IMAGE} AS apt-cache
# Populate APT cache w/ the fresh metadata and prefetch packages.
# Use an empty `docker-clean` file to "hide" the image-provided
# file to disallow removing packages after `apt-get` operations.
RUN --mount=type=tmpfs,target=/var/log \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    apt-get update \
 && apt-get --download-only -y install $(grep -h '^[^#]\+$' /root/*.lst)


FROM ${BASE_IMAGE} AS iwyu-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=install_iwyu.sh,target=/root/install_iwyu.sh \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_iwyu.sh


FROM ${BASE_IMAGE} AS rvm-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=install_rvm.sh,target=/root/install_rvm.sh \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_rvm.sh


FROM ${BASE_IMAGE}
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=install_deps.sh,target=/root/install_deps.sh \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=dpkg-exclude,target=/etc/dpkg/dpkg.cfg.d/exclude \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_deps.sh

RUN --mount=type=bind,from=iwyu-build,source=/root,target=/root \
    tar -C / -xf /root/iwyu.tar \
 && ln -s /usr/lib/llvm-15/bin/include-what-you-use /usr/bin/include-what-you-use-15

RUN --mount=type=bind,from=rvm-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rvm.tar
