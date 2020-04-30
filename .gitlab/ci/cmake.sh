#!/bin/sh

set -e

readonly version="3.17.2"
readonly sha256sum="dc57f3cc448ca67fc8776b4ad4c22b087b9c6a8e459938b9622b8c7f4ef6b21e"
readonly filename="cmake-$version-Linux-x86_64"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$version/$tarball"
sha256sum --check cmake.sha256sum
tar xf "$tarball"
mv "$filename" cmake
