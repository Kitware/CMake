#!/bin/sh

set -e

readonly version="1.10.0"
readonly sha256sum="6566836ddf3d72ca06685b34814e0c6fa0f0943542d651d0dab3150f10307c82"
readonly filename="ninja-linux"
readonly tarball="$filename.zip"

cd .gitlab

echo "$sha256sum  $tarball" > ninja.sha256sum
curl -OL "https://github.com/ninja-build/ninja/releases/download/v$version/$tarball"
sha256sum --check ninja.sha256sum
./cmake/bin/cmake -E tar xf "$tarball"
