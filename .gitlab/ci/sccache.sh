#!/bin/sh

set -e

readonly version="0.2.12"
readonly sha256sum="26fd04c1273952cc2a0f359a71c8a1857137f0ee3634058b3f4a63b69fc8eb7f"
readonly filename="sccache-$version-x86_64-unknown-linux-musl"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > sccache.sha256sum
curl -OL "https://github.com/mozilla/sccache/releases/download/$version/$tarball"
sha256sum --check sccache.sha256sum
tar xf "$tarball"
mv "$filename/sccache" .
