#!/bin/sh

set -e

readonly kernel="$(uname -s)-$(uname -m)"
case $kernel in
    Linux-x86_64)
        version="0.2.13"
        shatool="sha256sum"
        sha256sum="28a5499e340865b08b632306b435913beb590fbd7b49a3f887a623b459fabdeb"
        platform="x86_64-unknown-linux-musl"
        ;;
    Linux-aarch64)
        version="g6628e1f"
        shatool="sha256sum"
        sha256sum="bb88adbb5a29c166ecaa78d0593493b609a7f84d91d1228502a908f319b513f0"
        platform="aarch64-unknown-linux-musl"
        url="https://github.com/hwinit/sccache/releases/download/$version"
        ;;
    Darwin-x86_64)
        version="gfe63078"
        shatool="shasum -a 256"
        sha256sum="60a0302b1d7227f7ef56abd82266353f570d27c6e850c56c6448bf62def38888"
        platform="x86_64-apple-darwin"
        url="https://paraview.org/files/dependencies"
        ;;
    *)
        echo "Unrecognized platform $kernel"
        exit 1
        ;;
esac
readonly version
readonly shatool
readonly sha256sum
readonly platform

readonly filename="sccache-$version-$platform"
readonly tarball="$filename.tar.gz"

if [ -z "$url" ]; then
    url="https://github.com/mozilla/sccache/releases/download/$version"
fi
readonly url

cd .gitlab

echo "$sha256sum  $tarball" > sccache.sha256sum
curl -OL "$url/$tarball"
$shatool --check sccache.sha256sum
tar xf "$tarball"
mv "$filename/sccache" .
