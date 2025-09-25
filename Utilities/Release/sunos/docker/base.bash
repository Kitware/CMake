#!/usr/bin/env bash

set -e

arch="$1"
readonly arch

case "$arch" in
    x86_64)
        tarball="gcc-9.5.0-linux-x86_64-cross-sunos-x86_64.tar.xz"
        sha256sum="0632342dc20445a798148548376f986f3a09dc2e4f433fa9100e4a5371a14860"
        ;;
    sparc64)
        tarball="gcc-9.5.0-linux-x86_64-cross-sunos-sparc64.tar.xz"
        sha256sum="ea3c3deecdd94823edd7241aa4b79a0dc4e7fb5a8dc9d101cc2d6a72beab7ced"
        ;;
    *)
        echo >&2 "Unknown architecture: $arch"
        exit 1
        ;;
esac
readonly tarball
readonly sha256sum

cd /tmp

curl -OL "https://gitlab.kitware.com/api/v4/projects/6955/packages/generic/gcc-solaris/v9.5.0-20250227.0/$tarball"
echo "$sha256sum  $tarball" > gcc.sha256sum
sha256sum --check gcc.sha256sum

tar xJf "$tarball" -C /
