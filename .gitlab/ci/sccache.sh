#!/bin/sh

set -e

readonly kernel="$(uname -s)-$(uname -m)"
case $kernel in
    Linux-x86_64)
        version="v0.2.15"
        shatool="sha256sum"
        sha256sum="e5d03a9aa3b9fac7e490391bbe22d4f42c840d31ef9eaf127a03101930cbb7ca"
        platform="x86_64-unknown-linux-musl"
        ;;
    Linux-aarch64)
        version="v0.2.15"
        shatool="sha256sum"
        sha256sum="90d91d21a767e3f558196dbd52395f6475c08de5c4951a4c8049575fa6894489"
        platform="aarch64-unknown-linux-musl"
        ;;
    Darwin-x86_64)
        version="gfe63078"
        shatool="shasum -a 256"
        sha256sum="60a0302b1d7227f7ef56abd82266353f570d27c6e850c56c6448bf62def38888"
        platform="x86_64-apple-darwin"
        url="https://paraview.org/files/dependencies"
        ;;
    Darwin-arm64)
        version="0.2.15-1-disk_cache_init"
        shatool="shasum -a 256"
        sha256sum="f7c9ff78e701810b8b1dbc2a163c7fda1177fc3f69c71f46e7a38242657a99fd"
        platform="aarch64-apple-darwin"
        url="https://cmake.org/files/dependencies/sccache"
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
chmod +x sccache
