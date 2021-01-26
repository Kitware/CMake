#!/bin/sh

set -e

readonly version="3.19.3"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="c18b65697e9679e5c88dccede08c323cd3d3730648e59048047bba82097e0ffc"
        platform="Linux-x86_64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="a6b79ad05f89241a05797510e650354d74ff72cc988981cdd1eb2b3b2bda66ac"
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
