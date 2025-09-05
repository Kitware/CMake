#!/bin/sh

set -e

readonly version="1.9.0.20250814"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="6414d395eafee09453d2e203d9cc65f867e6ff7e1a8a6c08e444d86cb1d106ad"
        filename="appimagetool-$version-x86_64"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

cd .gitlab

# This URL is only visible inside of Kitware's network.  See above filename table.
baseurl="https://cmake.org/files/dependencies/internal"

tarball="$filename.tar.gz"
echo "$sha256sum  $tarball" > appimagetool.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check appimagetool.sha256sum
tar xzf "$tarball"
rm "$tarball" appimagetool.sha256sum
mv "$filename" "appimagetool"
