# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce an image containing a portable CMake binary package for Linux/x86_64.
# Build using the CMake source directory as the build context.
# The resulting image will have an '/out' directory containing the package.

ARG FROM_IMAGE_NAME=kitware/cmake:build-linux-x86_64-deps-2019-08-09
ARG FROM_IMAGE_DIGEST=@sha256:630c320b26a67fc584e0bc98314f1fb0cb0abc764348bb2613ef07437f7101f9
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
 && export LANG=en_US.UTF-8 \
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
 && mv cmake-*-Linux-x86_64.* /out \
 && :
