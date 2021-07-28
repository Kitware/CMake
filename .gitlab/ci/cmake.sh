#!/bin/sh

set -e

readonly version="3.21.0"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="d54ef6909f519740bc85cec07ff54574cd1e061f9f17357d9ace69f61c6291ce"
        platform="linux-x86_64"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="b1e46825bf370f45f8f47c3a497b1122759ee41fbd60187e525f517a4b0934eb"
        platform="linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="c1c6f19dfc9c658a48b5aed22806595b2337bb3aedb71ab826552f74f568719f"
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
