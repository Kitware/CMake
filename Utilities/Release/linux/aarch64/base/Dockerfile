# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Produce a base image with a build environment for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=centos:8
ARG FROM_IMAGE_DIGEST=@sha256:a27fd8080b517143cbbbab9dfb7c8571c40d67d534bbdee55bd6c473f432b177
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

COPY repo-vault.sed /root/

RUN : \
 && sed -f /root/repo-vault.sed -i /etc/yum.repos.d/*.repo \
 && sed -i '/^enabled=0/s/0/1/' /etc/yum.repos.d/CentOS-Linux-PowerTools.repo \
 && dnf install -y \
        ca-certificates \
        curl \
        fontconfig-devel \
        freetype-devel \
        gcc \
        gcc-c++ \
        git \
        libstdc++-static \
        libX11-devel \
        libxcb-devel \
        make \
        patch \
        perl \
        perl-core \
        python3-pip \
        xz \
        which \
 && dnf clean all \
 && :
