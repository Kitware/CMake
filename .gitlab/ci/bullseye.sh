#!/bin/sh

set -e

if ! test -f "$CMAKE_CI_BULLSEYE_LICENSE"; then
    echo "No CMAKE_CI_BULLSEYE_LICENSE file provided!"
    exit 1
fi

readonly version="9.22.3"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        tarball="BullseyeCoverage-$version-Linux-x64.tar.xz"
        sha256sum="d5be7e65d9363161b67fa77a30407c7c200d995af79a422c4e2e278802ba0776"
        shatool="sha256sum"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

# See https://www.bullseye.com/download-archive for original archives.
# This URL is only visible inside of Kitware's network.
baseurl="https://cmake.org/files/dependencies/internal/bullseye"

dirname="BullseyeCoverage-$version"
echo "$sha256sum  $tarball" > bullseye.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check bullseye.sha256sum
tar xJf "$tarball"
"$dirname/install" --key "$(<"$CMAKE_CI_BULLSEYE_LICENSE")" --prefix=/opt/bullseye
rm -r "$dirname" "$tarball" bullseye.sha256sum "$CMAKE_CI_BULLSEYE_LICENSE"
