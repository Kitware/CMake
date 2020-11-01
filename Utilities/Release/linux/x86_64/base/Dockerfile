# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce a base image with a build environment for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=centos:6
ARG FROM_IMAGE_DIGEST=@sha256:dec8f471302de43f4cfcf82f56d99a5227b5ea1aa6d02fa56344986e1f4610e7
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

RUN : \
 && yum install -y centos-release-scl \
 && yum install -y \
        ca-certificates \
        curl \
        devtoolset-6-gcc \
        devtoolset-6-gcc-c++ \
        fontconfig-devel \
        freetype-devel \
        git \
        libX11-devel \
        libxcb-devel \
        make \
        patch \
        perl \
        rh-python36-python-pip \
        xz \
 && yum clean all \
 && :
