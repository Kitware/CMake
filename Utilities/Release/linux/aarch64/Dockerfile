# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Produce an image containing a portable CMake binary package for Linux/aarch64.
# Build using the CMake source directory as the build context.
# The resulting image will have an '/out' directory containing the package.

# Keep this in sync with the `.gitlab/os-linux.yml` `.linux_release_aarch64` image.
ARG FROM_IMAGE_NAME=kitware/cmake:build-linux-aarch64-deps-2026-11-20
ARG FROM_IMAGE_DIGEST=@sha256:a3c949f132550a3818b03eba7e7cf3509d9db478169e0db9bbc6eac0d99d8d69
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

COPY . /opt/cmake/src/cmake

ARG TEST=true

RUN : \
 && mkdir -p /opt/cmake/src/cmake-build \
 && cd /opt/cmake/src/cmake-build \
 && cp ../cmake/Utilities/Release/linux/aarch64/cache.txt CMakeCache.txt \
 && source /opt/rh/gcc-toolset-11/enable \
 && set -x \
 && ../cmake/bootstrap --parallel=$(nproc) --docdir=doc/cmake \
 && nice make -j $(nproc) \
 && if $TEST; then \
        # Run tests that require the full build tree.
        bin/ctest --output-on-failure -j 8 -R '^(CMake\.|CMakeLib\.|RunCMake\.ctest_memcheck)'; \
    fi \
 && bin/cpack -G TGZ \
 && bin/cpack -G STGZ \
 && set +x \
 && mkdir /out \
 && mv cmake-*-linux-aarch64.* /out \
 && :
