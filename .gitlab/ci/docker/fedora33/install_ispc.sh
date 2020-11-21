#!/bin/sh

set -e

readonly version="1.13.0"
readonly sha256sum="8ab1189bd5db596b3eee9d9465d3528b6626a7250675d67102761bb0d284cd21"

readonly filename="ispc-v$version-linux"
readonly tarball="$filename.tar.gz"

echo "$sha256sum  $tarball" > ispc.sha256sum
curl -OL "https://github.com/ispc/ispc/releases/download/v$version/$tarball"
sha256sum --check ispc.sha256sum
tar --strip-components=1 -C /usr/local -xf "$tarball" "$filename/bin/ispc"
