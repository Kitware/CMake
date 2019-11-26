# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce a base image with a test environment for packaged CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=debian:9
ARG FROM_IMAGE_DIGEST=@sha256:397b2157a9ea8d7f16c613aded70284292106e8b813fb1ed5de8a8785310a26a
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

RUN : \
 && apt-get update \
 && apt-get install -y \
        dpkg \
        file \
        gcc \
        g++ \
        gfortran \
        qt5-default \
        make \
        ninja-build \
 && apt-get clean \
 && :

COPY test-make.bash test-ninja.bash /
