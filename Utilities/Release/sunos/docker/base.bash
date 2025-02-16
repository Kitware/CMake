#!/usr/bin/env bash

set -e

arch="$1"
readonly arch

case "$arch" in
    i386)
        tarball="gcc-9.5.0-linux-x86_64-cross-sunos-i386.tar.xz"
        sha256sum="3cd3c989483051e741dd9f39170842d22e5c43cd25628d2b0c57890a3f235883"
        ;;
    sparc)
        tarball="gcc-9.5.0-linux-x86_64-cross-sunos-sparc.tar.xz"
        sha256sum="853454ef4e787895786fdb21e56a3ba9c121ffe6116467a75f2c3eb09f3c88b4"
        ;;
    *)
        echo >&2 "Unknown architecture: $arch"
        exit 1
        ;;
esac
readonly tarball
readonly sha256sum

cd /tmp

curl -OL "https://gitlab.kitware.com/api/v4/projects/6955/packages/generic/gcc-solaris/v9.5.0-20250212.0/$tarball"
echo "$sha256sum  $tarball" > gcc.sha256sum
sha256sum --check gcc.sha256sum

tar xJf "$tarball" -C /
