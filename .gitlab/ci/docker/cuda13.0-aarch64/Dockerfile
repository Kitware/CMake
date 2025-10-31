# syntax=docker/dockerfile:1

ARG BASE_IMAGE=kitware/nvidia-cuda:13.0.1-devel-ubuntu24.04

FROM ${BASE_IMAGE} AS apt-config
ADD https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/sbsa/cuda-keyring_1.1-1_all.deb /root/
RUN --mount=type=tmpfs,target=/var/log \
    dpkg -i /root/cuda-keyring_1.1-1_all.deb \
 && rm /root/cuda-keyring_1.1-1_all.deb /etc/apt/sources.list.d/cuda.list

FROM apt-config AS apt-cache
# Populate APT cache w/ the fresh metadata and prefetch packages.
# Use an empty `docker-clean` file to "hide" the image-provided
# file to disallow removing packages after `apt-get` operations.
RUN --mount=type=tmpfs,target=/var/log \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    apt-get update \
 && apt-get --download-only -y install $(grep -h '^[^#]\+$' /root/*.lst)

FROM apt-config
MAINTAINER Brad King <brad.king@kitware.com>

RUN --mount=type=bind,source=install_deps.sh,target=/root/install_deps.sh \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=dpkg-exclude,target=/etc/dpkg/dpkg.cfg.d/exclude \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_deps.sh
