# syntax=docker/dockerfile:1

ARG BASE_IMAGE=rocm/dev-ubuntu-22.04:5.5

FROM ${BASE_IMAGE} AS cuda-keyring
ADD https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb /root/
RUN --mount=type=tmpfs,target=/var/log \
    dpkg -i /root/cuda-keyring_1.1-1_all.deb \
 && rm /root/cuda-keyring_1.1-1_all.deb

FROM cuda-keyring AS apt-cache
# Populate APT cache w/ the fresh metadata and prefetch packages.
# Use an empty `docker-clean` file to "hide" the image-provided
# file to disallow removing packages after `apt-get` operations.
RUN --mount=type=tmpfs,target=/var/log \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    apt-get update \
 && apt-get --download-only -y install $(grep -h '^[^#]\+$' /root/*.lst)

FROM cuda-keyring
MAINTAINER Brad King <brad.king@kitware.com>

ENV NVIDIA_DRIVER_CAPABILITIES=compute,utility
ENV NVIDIA_REQUIRE_CUDA=cuda>=11.8
ENV NVIDIA_VISIBLE_DEVICES=all
ENV PATH="/opt/rocm/bin:$PATH"

RUN --mount=type=bind,source=install_deps.sh,target=/root/install_deps.sh \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=dpkg-exclude,target=/etc/dpkg/dpkg.cfg.d/exclude \
    --mount=type=bind,source=docker-clean,target=/etc/apt/apt.conf.d/docker-clean \
    --mount=type=cache,from=apt-cache,source=/var/lib/apt/lists,target=/var/lib/apt/lists \
    --mount=type=cache,from=apt-cache,source=/var/cache/apt,target=/var/cache/apt,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_deps.sh
