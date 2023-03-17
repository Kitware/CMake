#!/bin/sh

set -e

baseurl="https://cmake.org/files/dependencies/openmp"

case "$(uname -s)-$(uname -m)" in
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="6eef660db7a085a04f87e4aac79da9f37d26ff0fb17c8781d3a21bd5244997e9"
        filename="openmp-12.0.1-darwin20-Release"
        # tarball contains usr/local/
        strip_components=--strip-components=2
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

readonly tarball="$filename.tar.gz"

cd .gitlab
mkdir -p openmp

echo "$sha256sum  $tarball" > openmp.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check openmp.sha256sum
tar -C openmp $strip_components -xzf $tarball
rm $tarball openmp.sha256sum
