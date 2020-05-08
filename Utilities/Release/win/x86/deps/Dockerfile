# escape=`

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce an image with custom-built dependencies for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=kitware/cmake:build-win-x86-base-2020-04-27
ARG FROM_IMAGE_DIGEST=@sha256:c5a8948d636319cdac0180266996558bb6fb037125792b5b837f069d02e53d7c
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST

# Qt Source
FROM $FROM_IMAGE AS qt-src

# JOM
ADD http://download.qt-project.org/official_releases/jom/unstable-jom.zip C:\jom\jom.zip
RUN cd \jom `
 && powershell -Command " `
    if ($(Get-FileHash jom.zip).Hash -eq '128fdd846fe24f8594eed37d1d8929a0ea78df563537c0c1b1861a635013fff8') {`
      Expand-Archive -Path jom.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del jom.zip

# XZ
ADD https://tukaani.org/xz/xz-5.2.5-windows.zip C:\xz\xz.zip
RUN cd \xz `
 && powershell -Command " `
    if ($(Get-FileHash xz.zip).Hash -eq 'd83b82ca75dfab39a13dda364367b34970c781a9df4d41264db922ac3a8f622d') {`
      Expand-Archive -Path xz.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del xz.zip

# Git
ADD https://github.com/git-for-windows/git/releases/download/v2.26.2.windows.1/MinGit-2.26.2-busybox-64-bit.zip C:\git\git.zip
RUN cd \git `
 && powershell -Command " `
    if ($(Get-FileHash git.zip).Hash -eq 'e834ea73fe093fb180dc45f67a1f2a7a566dab53d1d45bc3cd150106f5c40520') {`
      Expand-Archive -Path git.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del git.zip

# Qt Source
ADD https://download.qt.io/official_releases/qt/5.12/5.12.1/single/qt-everywhere-src-5.12.1.tar.xz C:\qt-src\qt.tar.xz
RUN cd \qt-src `
 && powershell -Command " `
    if ($(Get-FileHash qt.tar.xz).Hash -eq 'caffbd625c7bc10ff8c5c7a27dbc7d84fa4de146975c0e1ffe904b514ccd6da4') {`
      \xz\bin_x86-64\xz -d qt.tar.xz `
    } else {`
      exit 1 `
    }" `
 && tar xvf qt.tar `
 && del qt.tar `
 && move qt-everywhere-src-5.12.1 qt
COPY qt-build.bat qt-install.patch C:\qt-src\

# Qt Build i386
FROM qt-src as qt-i386
RUN \qt-src\qt-build.bat i386

# Qt Build x86_64
FROM qt-src as qt-x86_64
RUN \qt-src\qt-build.bat x86_64

# Output Stage
FROM $FROM_IMAGE

# Qt
COPY --from=qt-i386 C:\qt-i386 C:\qt-i386
COPY --from=qt-x86_64 C:\qt-x86_64 C:\qt-x86_64

# WIX
ADD https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311-binaries.zip C:\wix\wix.zip
RUN cd \wix `
 && powershell -Command " `
    if ($(Get-FileHash wix.zip).Hash -eq '2c1888d5d1dba377fc7fa14444cf556963747ff9a0a289a3599cf09da03b9e2e') {`
      Expand-Archive -Path wix.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del wix.zip

# Python and Sphinx
ADD https://www.python.org/ftp/python/3.8.2/python-3.8.2-embed-amd64.zip C:\python3\python3.zip
RUN cd \python3 `
 && powershell -Command " `
    if ($(Get-FileHash python3.zip).Hash -eq '2927a3a6d0fe1f6e047a86059220aeda374eed23113b9ef5355acb8452d56453') {`
      Expand-Archive -Path python3.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && del python3.zip `
 && curl -O https://bootstrap.pypa.io/get-pip.py `
 && python get-pip.py `
 && del python38._pth `
 && set "PY_LIBS=C:\python3\Lib;C:\Python3\Lib\site-packages" `
 && set "PY_PIP=C:\python3\Scripts" `
 && Scripts\pip install --no-warn-script-location sphinx==2.1.2

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

# CMake
ADD https://github.com/Kitware/CMake/releases/download/v3.17.1/cmake-3.17.1-win64-x64.zip C:\cmake\cmake.zip
RUN cd \cmake `
 && powershell -Command " `
    if ($(Get-FileHash cmake.zip).Hash -eq 'a5af7a2fe73f34070456397e940042e4469f072126c82974f44333ac43d478b1') {`
      Expand-Archive -Path cmake.zip -DestinationPath .`
    } else {`
      exit 1 `
    }" `
 && move cmake-*-win64-x64 cmake `
 && del cmake.zip
