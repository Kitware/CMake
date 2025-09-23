# syntax=docker/dockerfile:1

ARG BASE_IMAGE=debian:12

FROM ${BASE_IMAGE} AS apt-cache
# Populate APT cache w/ the fresh metadata and prefetch packages.
# Use an empty `docker-clean` file to "hide" the image-provided
# file to disallow removing packages after `apt-get` operations.
RUN --mount=type=tmpfs,target=/var/log \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=bind,source=base.lst,target=/root/base.lst \
    --mount=type=bind,source=openssl.lst,target=/root/openssl.lst \
    apt-get update \
 && apt-get --download-only -y install $(grep -h '^[^#]\+$' /root/*.lst)

FROM ${BASE_IMAGE} AS base
ARG ARCH=sparc64
RUN --mount=type=bind,source=base.lst,target=/root/base.lst \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    apt-get install -y $(grep '^[^#]\+$' /root/base.lst)
RUN --mount=type=bind,source=base.bash,target=/root/base.bash \
    --mount=type=tmpfs,target=/tmp \
    /root/base.bash $ARCH

FROM base AS sysroot
ARG SYSROOT_URL
ARG SYSROOT_SHA256SUM
RUN --mount=type=bind,source=sysroot.bash,target=/root/sysroot.bash \
    --mount=type=tmpfs,target=/tmp \
    /root/sysroot.bash $ARCH

FROM sysroot AS openssl
RUN --mount=type=bind,source=openssl.lst,target=/root/openssl.lst \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    apt-get install -y $(grep '^[^#]\+$' /root/openssl.lst)
RUN --mount=type=bind,source=openssl.bash,target=/root/openssl.bash \
    --mount=type=bind,source=openssl.patch,target=/root/openssl.patch \
    --mount=type=bind,from=sysroot,source=/opt/cross/sysroot,target=/opt/cross/sysroot \
    --mount=type=tmpfs,target=/tmp \
    /root/openssl.bash $ARCH

FROM base
LABEL maintainer="Brad King <brad.king@kitware.com>"
RUN --mount=type=bind,from=openssl,source=/root,target=/root \
    tar xzf /root/openssl.tar.gz -C /
