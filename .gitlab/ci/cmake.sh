#!/bin/sh

set -e

readonly version="3.17.2"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="dc57f3cc448ca67fc8776b4ad4c22b087b9c6a8e459938b9622b8c7f4ef6b21e"
        platform="Linux"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="139500e20b080444fcafe57f24f57248c691c5187cce6695bee2b9aad6792c7d"
        platform="Darwin"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="cmake-$version-$platform-x86_64"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$version/$tarball"
$shatool --check cmake.sha256sum
tar xf "$tarball"
mv "$filename" cmake

if [ "$( uname -s )" = "Darwin" ]; then
    ln -s CMake.app/Contents/bin cmake/bin
fi
