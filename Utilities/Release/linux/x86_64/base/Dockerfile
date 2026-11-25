# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Produce a base image with a build environment for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=centos:7
ARG FROM_IMAGE_DIGEST=@sha256:be65f488b7764ad3638f236b7b515b3678369a5124c47b8d32916d6487418ea4
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST

FROM $FROM_IMAGE
LABEL maintainer="Brad King <brad.king@kitware.com>"

COPY repo-vault.sed /root/

RUN : \
 && sed -f /root/repo-vault.sed -i /etc/yum.repos.d/*.repo \
 && yum install -y \
        centos-release-scl \
        epel-release \
 && sed -f /root/repo-vault.sed -i /etc/yum.repos.d/*.repo \
 && yum install -y \
        ca-certificates \
        curl \
        devtoolset-11-gcc \
        devtoolset-11-gcc-c++ \
        devtoolset-11-make \
        fontconfig-devel \
        freetype-devel \
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
 && yum clean all \
 && :
