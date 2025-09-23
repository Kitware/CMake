# syntax=docker/dockerfile:1

ARG BASE_IMAGE=fedora:42

FROM ${BASE_IMAGE} AS dnf-cache
LABEL maintainer="Brad King <brad.king@kitware.com>"
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=cache,target=/var/lib/dnf \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install -y $(grep -h '^[^#]\+$' /root/*.lst)
