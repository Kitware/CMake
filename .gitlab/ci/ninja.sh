#!/bin/sh

set -e

readonly version="1.10.0"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="6566836ddf3d72ca06685b34814e0c6fa0f0943542d651d0dab3150f10307c82"
        platform="linux"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="2ee405c0e205d55666c60cc9c0d8d04c8ede06d3ef2e2c2aabe08fd81c17d22e"
        platform="mac"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="ninja-$platform"
readonly tarball="$filename.zip"

cd .gitlab

echo "$sha256sum  $tarball" > ninja.sha256sum
curl -OL "https://github.com/ninja-build/ninja/releases/download/v$version/$tarball"
$shatool --check ninja.sha256sum
./cmake/bin/cmake -E tar xf "$tarball"
