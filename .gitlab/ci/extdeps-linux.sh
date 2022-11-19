#!/bin/sh

set -e

mkdir -p /opt/extdeps/src
cd /opt/extdeps/src
export PATH=/opt/extdeps/bin:$PATH

#----------------------------------------------------------------------------
# cmake

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        cmake_version="3.13.5"
        cmake_sha256sum="e2fd0080a6f0fc1ec84647acdcd8e0b4019770f48d83509e6a5b0b6ea27e5864"
        cmake_platform="Linux-x86_64"
        ;;
    Linux-aarch64)
        cmake_version="3.19.8"
        cmake_sha256sum="807f5afb2a560e00af9640e496d5673afefc2888bf0ed076412884a5ebb547a1"
        cmake_platform="Linux-aarch64"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly cmake_sha256sum
readonly cmake_platform

readonly cmake_filename="cmake-$cmake_version-$cmake_platform"
readonly cmake_tarball="$cmake_filename.tar.gz"

echo "$cmake_sha256sum  $cmake_tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$cmake_version/$cmake_tarball"
sha256sum --check cmake.sha256sum
tar xzf "$cmake_tarball" -C /opt/extdeps --strip-components=1
rm -f "$cmake_tarball" cmake.sha256sum

#----------------------------------------------------------------------------
# libuv

curl -L -o libuv-1.28.0.tar.gz https://github.com/libuv/libuv/archive/refs/tags/v1.28.0.tar.gz
tar xzf libuv-1.28.0.tar.gz
cmake -S libuv-1.28.0 -B libuv-1.28.0-build \
  -DCMAKE_INSTALL_PREFIX=/opt/extdeps
cmake --build libuv-1.28.0-build --target install
rm -rf libuv-1.28.0*

#----------------------------------------------------------------------------
# jsoncpp

curl -L -o jsoncpp-1.6.0.tar.gz https://github.com/open-source-parsers/jsoncpp/archive/refs/tags/1.6.0.tar.gz
tar xzf jsoncpp-1.6.0.tar.gz
cmake -S jsoncpp-1.6.0 -B jsoncpp-1.6.0-build \
  -DCMAKE_BUILD_TYPE=Release \
  -DJSONCPP_LIB_BUILD_STATIC=ON \
  -DJSONCPP_LIB_BUILD_SHARED=ON \
  -DCMAKE_INSTALL_PREFIX=/opt/extdeps
cmake --build jsoncpp-1.6.0-build --target install
rm -rf jsoncpp-1.6.0*
