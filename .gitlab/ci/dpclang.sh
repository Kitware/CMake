#!/bin/sh

set -e

baseurl="https://github.com/intel/llvm/releases/download/nightly-2026-10-11/$tarball"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        tarball="sycl_linux.tar.gz"
        shatool="sha256sum"
        sha256sum="5b0ed70b8c8ad551462c932caaf9377e8cae3cc67dbe14772d119eefe8e9e394"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

cd .gitlab

echo "$sha256sum  $tarball" > dpclang.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check dpclang.sha256sum
mkdir dpclang
tar -C dpclang -xzf "$tarball"
rm "$tarball" dpclang.sha256sum
