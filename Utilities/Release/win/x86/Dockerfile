# escape=`

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce an image containing a portable CMake binary package for Windows.
# Build using the CMake source directory as the build context.
# The resulting image will have a 'c:\out' directory containing the package.

ARG FROM_IMAGE_NAME=kitware/cmake:build-win-x86-deps-2020-04-27
ARG FROM_IMAGE_DIGEST=@sha256:04e229c0c0ba2247855d0e8c0fb87c1686f983adbafa4ce413e61b3905edb76b
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST

FROM $FROM_IMAGE as build
COPY . C:\cmake\src\cmake
ARG ARCH="x86_64"
ARG TEST="true"
RUN \cmake\src\cmake\Utilities\Release\win\x86\build.bat %ARCH% %TEST%

# Package in a separate stage so the builder can optionally skip it.
FROM build as pack
ARG PACK="ZIP WIX"
RUN \cmake\src\cmake\Utilities\Release\win\x86\pack.bat %PACK%
