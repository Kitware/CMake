# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Produce a base image with a build environment for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=centos:8
ARG FROM_IMAGE_DIGEST=@sha256:a27fd8080b517143cbbbab9dfb7c8571c40d67d534bbdee55bd6c473f432b177
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST

FROM $FROM_IMAGE
LABEL maintainer="Brad King <brad.king@kitware.com>"

COPY repo-vault.sed /root/

RUN : \
 && sed -f /root/repo-vault.sed -i /etc/yum.repos.d/*.repo \
 && sed -i '/^enabled=0/s/0/1/' /etc/yum.repos.d/CentOS-Linux-PowerTools.repo \
 && dnf install -y epel-release \
 && dnf install -y \
        ca-certificates \
        curl \
        fontconfig-devel \
        freetype-devel \
        gcc-toolset-11-gcc \
        gcc-toolset-11-gcc-c++ \
        gcc-toolset-11-make \
        git \
        libX11-devel \
        libxcb-devel \
        libxkbcommon-x11-devel \
        patch \
        perl \
        perl-core \
        perl-IPC-Cmd \
        perl-Time-Piece \
        unzip \
        which \
        xcb-util-cursor-devel  \
        xcb-util-devel \
        xcb-util-image-devel \
        xcb-util-keysyms-devel \
        xcb-util-renderutil-devel \
        xcb-util-wm-devel \
        xz \
        zip \
 && dnf clean all \
 && :
