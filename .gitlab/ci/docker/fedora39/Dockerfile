# syntax=docker/dockerfile:1

ARG BASE_IMAGE=fedora:39

FROM ${BASE_IMAGE} AS dnf-cache
# Populate DNF cache w/ the fresh metadata and prefetch packages.
RUN --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install \
        --setopt=install_weak_deps=False \
        --setopt=fastestmirror=True \
        --setopt=max_parallel_downloads=10 \
        --downloadonly \
        -y \
        $(grep -h '^[^#]\+$' /root/*.lst)


FROM ${BASE_IMAGE} AS rvm-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=install_rvm.sh,target=/root/install_rvm.sh \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/dnf,target=/var/cache/dnf,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_rvm.sh


FROM ${BASE_IMAGE} AS iwyu-build
LABEL maintainer="Kyle Edwards <kyle.edwards@kitware.com>"

RUN --mount=type=bind,source=install_iwyu.sh,target=/root/install_iwyu.sh \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/dnf,target=/var/cache/dnf,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_iwyu.sh


FROM ${BASE_IMAGE}
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=install_deps.sh,target=/root/install_deps.sh \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/dnf,target=/var/cache/dnf,sharing=private \
    --mount=type=cache,target=/var/cache/pip \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_deps.sh

RUN --mount=type=bind,from=rvm-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rvm.tar

RUN --mount=type=bind,from=iwyu-build,source=/root,target=/root \
    tar -C / -xf /root/iwyu.tar
