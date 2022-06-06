#!/bin/sh

set -e

readonly version="1.18.0"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="6c379bb97962e9de7d24fd48b3f7e647dc42be898e9d187948220268c646b692"
        platform="linux"
        ;;
    Darwin-x86_64)
        shatool="shasum -a 256"
        sha256sum="d1435b541182406ff6b18446d31ecceef0eae3aed7654391ae676d3142e0000d"
        platform="macOS"
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
