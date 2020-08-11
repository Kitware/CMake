#!/bin/sh

set -e

readonly version="0.2.13"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="28a5499e340865b08b632306b435913beb590fbd7b49a3f887a623b459fabdeb"
        platform="x86_64-unknown-linux-musl"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="f564e948abadfc9e409eb1cd7babf24c6784057d5506c3b0a04cdd37cd830ae9"
        platform="x86_64-apple-darwin"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="sccache-$version-$platform"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > sccache.sha256sum
curl -OL "https://github.com/mozilla/sccache/releases/download/$version/$tarball"
$shatool --check sccache.sha256sum
tar xf "$tarball"
mv "$filename/sccache" .
