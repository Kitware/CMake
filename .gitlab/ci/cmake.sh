#!/bin/sh

set -e

readonly version="3.18.4"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="149e0cee002e59e0bb84543cf3cb099f108c08390392605e944daeb6594cbc29"
        platform="Linux"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="9d27049660474cf134ab46fa0e0db771b263313fcb8ba82ee8b2d1a1a62f8f20"
        platform="Darwin"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="cmake-$version-$platform-x86_64"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$version/$tarball"
$shatool --check cmake.sha256sum
tar xf "$tarball"
mv "$filename" cmake

if [ "$( uname -s )" = "Darwin" ]; then
    ln -s CMake.app/Contents/bin cmake/bin
fi
