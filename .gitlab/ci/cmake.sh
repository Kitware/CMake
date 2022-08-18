#!/bin/sh

set -e

readonly version="3.24.1"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="827bf068cfaa23a9fb95f990c9f8a7ed8f2caeb3af62b5c0a2fed7a8dd6dde3e"
        platform="linux-x86_64"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="d50c40135df667ed659f8e4eb7cf7d53421250304f7b3e1a70af9cf3d0f2ab18"
        platform="linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="71bb8db69826d74c395a3c3bbf8b773dbe9f54a2c7331266ba70da303e9c97a1"
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
