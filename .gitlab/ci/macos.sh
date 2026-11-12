#!/bin/sh

set -e

# This URL is only visible inside of Kitware's network.
baseurl="https://cmake.org/files/dependencies/internal/macos"

case "$(uname -s)-$(uname -m)" in
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="427612880d6c40bcef2b0ecb39d92b057ee7a43ec3552fbd4449859991eb1cc6"
        tarball="MacOSX15.5.sdk.tar.bz2"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

cd .gitlab

echo "$sha256sum  $tarball" > macos.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check macos.sha256sum
tar xjf "$tarball"
rm "$tarball" macos.sha256sum
