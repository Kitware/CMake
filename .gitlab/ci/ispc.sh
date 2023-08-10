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
        sha256sum="62cee043a3a4dbff8c2f6d3885a7e573901bbc1325dd93d50f92904b7ea67fec"
        platform="macOS.arm64"
        ;;
    Darwin-x86_64)
        shatool="shasum -a 256"
        sha256sum="da0f11a048a316081a8ad8170d48b170b2ed7efc3b140fc88b8611238809c8e4"
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
