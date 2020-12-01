#!/bin/sh

set -e

readonly version="3.19.0"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="5446cdee900e906e46162a5a7be9b4542bb5e886041cf8cffeda62aae2696ccf"
        platform="Linux"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="315eb5500753f797075b6ea43189420e97b0b9f32c8fc87ec94ba1d80b72eb7f"
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
