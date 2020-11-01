# escape=`

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce a base image with a test environment for packaged CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=kitware/cmake:build-win-x86-base-2020-04-27
ARG FROM_IMAGE_DIGEST=@sha256:c5a8948d636319cdac0180266996558bb6fb037125792b5b837f069d02e53d7c
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

# Python
ADD https://www.python.org/ftp/python/3.8.2/python-3.8.2-embed-amd64.zip C:\python3\python3.zip
RUN cd \python3 `
 && powershell -Command " `
    if ($(Get-FileHash python3.zip).Hash -eq '2927a3a6d0fe1f6e047a86059220aeda374eed23113b9ef5355acb8452d56453') {`
      Expand-Archive -Path python3.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del python3.zip `
 && del python38._pth

# Ninja
ADD https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-win.zip C:\ninja\ninja.zip
RUN cd \ninja `
 && powershell -Command " `
    if ($(Get-FileHash ninja.zip).Hash -eq '919fd158c16bf135e8a850bb4046ec1ce28a7439ee08b977cd0b7f6b3463d178') {`
      Expand-Archive -Path ninja.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del ninja.zip

COPY test-nmake.bat test-ninja.bat C:\
