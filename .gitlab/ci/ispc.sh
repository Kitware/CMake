#!/bin/sh

set -e

readonly version="1.20.0"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="e6412b88aa312fcd10c46f92df0149ccc4d99e53552c4ce127aa6c634fe9b308"
        platform="linux"
        ;;
    Darwin-arm64)
        shatool="shasum -a 256"
        sha256sum="c423a5a88d7a9a6ed667e41d025801c123fa0c5fd384d4ea138fa1fcf2bc24c9"
        platform="macOS.arm64"
        ;;
    Darwin-x86_64)
        shatool="shasum -a 256"
        sha256sum="e25222d2d6f4f8e3561556ac73f88721ceb5486439d6c2a566d37407ad9a5907"
        platform="macOS.x86_64"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="ispc-v$version-$platform"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > ispc.sha256sum
curl -OL "https://github.com/ispc/ispc/releases/download/v$version/$tarball"
$shatool --check ispc.sha256sum
tar xf "$tarball"
mv "$filename" ispc
