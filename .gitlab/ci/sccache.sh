#!/bin/sh

set -e

readonly version="0.2.15-background-init"
readonly build_date="20210602.0"

case "$( uname -s )-$(uname -m)" in
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="28b9ad3f591874551a3f4c5c1ff32456d3328c15d7bd8bc63b4e5948a94f1def"
        platform="aarch64-unknown-linux-musl"
        ;;
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="34d62d30eae1a4145f00d62b01ad21c3456e28f11f8246c936b00cccf4855016"
        platform="x86_64-unknown-linux-musl"
        ;;
    Darwin-x86_64|Darwin-arm64)
        shatool="shasum -a 256"
        sha256sum="2fa396e98cc8d07e39429b187a77386db63d35409902251d462080bdd0087c22"
        platform="universal-apple-darwin"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )-$( uname -m )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="sccache-v$version-$platform"

readonly url="https://gitlab.kitware.com/api/v4/projects/6955/packages/generic/sccache/v$version-$build_date/"

cd .gitlab

echo "$sha256sum  $filename" > sccache.sha256sum
curl -OL "$url/$filename"
$shatool --check sccache.sha256sum
mv "$filename" sccache
chmod +x sccache
