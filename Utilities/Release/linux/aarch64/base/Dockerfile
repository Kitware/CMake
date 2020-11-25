# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce a base image with a build environment for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=centos:7
ARG FROM_IMAGE_DIGEST=@sha256:43964203bf5d7fe38c6fca6166ac89e4c095e2b0c0a28f6c7c678a1348ddc7fa
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

RUN : \
 && yum install -y centos-release-scl \
 && yum install -y \
        ca-certificates \
        curl \
        devtoolset-7-gcc \
        devtoolset-7-gcc-c++ \
        fontconfig-devel \
        freetype-devel \
        git \
        libX11-devel \
        libxcb-devel \
        make \
        patch \
        perl \
        python3-pip \
        xz \
        which \
 && yum clean all \
 && :
