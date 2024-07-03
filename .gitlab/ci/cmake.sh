#!/bin/sh

set -e

readonly version="3.30.0"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="09846a3858583f38189b59177586adf125a08c15f3cddcaf7d7d7081ac86969f"
        platform="linux-x86_64"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="3db7f12dadc91be9b30f0e5a71038c51d7f5b7cc03e32625621a052e9996bef0"
        platform="linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="5fd0550f6a1ce1b88276a6d828483e8ddbf5e14053b32a0e534e95294ba3a2e3"
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
