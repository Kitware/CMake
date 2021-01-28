# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce an image containing a portable CMake binary package for Linux/x86_64.
# Build using the CMake source directory as the build context.
# The resulting image will have an '/out' directory containing the package.

# Keep this in sync with the `.gitlab/os-linux.yml` `.linux_release_x86_64` image.
ARG FROM_IMAGE_NAME=kitware/cmake:build-linux-x86_64-deps-2020-04-02
ARG FROM_IMAGE_DIGEST=@sha256:77e9ab183f34680990db9da5945473e288f0d6556bce79ecc1589670d656e157
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

COPY . /opt/cmake/src/cmake

ARG TEST=true

RUN : \
 && mkdir -p /opt/cmake/src/cmake-build \
 && cd /opt/cmake/src/cmake-build \
 && cp ../cmake/Utilities/Release/linux/x86_64/cache.txt CMakeCache.txt \
 && source /opt/rh/devtoolset-6/enable \
 && source /opt/rh/rh-python36/enable \
 && set -x \
 && ../cmake/bootstrap --parallel=$(nproc) --docdir=doc/cmake \
 && nice make -j $(nproc) \
 && if $TEST; then \
        # Run tests that require the full build tree.
        bin/ctest --output-on-failure -j 8 -R '^(CMake\.|CMakeLib\.|CMakeServerLib\.|RunCMake\.ctest_memcheck)'; \
    fi \
 && bin/cpack -G TGZ \
 && bin/cpack -G STGZ \
 && set +x \
 && mkdir /out \
 && mv cmake-*-linux-x86_64.* /out \
 && :
