#!/bin/sh

set -e

readonly version="3.21.0-rc2"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="31d4f5f4bb8793127eb441c8311fd4baad4d2a86c26fd37b43d4ef23887c332e"
        platform="linux-x86_64"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="437565f32e3f28fd8322f96a5472b196825eaf5ef6139e1c383afe624decc476"
        platform="linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="5ab0ee4edb45de33080f571cf69300b2abd92dc02a9c624b3c289a4a14cab160"
        platform="macos-universal"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="cmake-$version-$platform"
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
