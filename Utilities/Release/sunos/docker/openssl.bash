#!/usr/bin/env bash

set -e

arch="$1"
readonly arch

case "$arch" in
    x86_64)
        target=x86_64-pc-solaris2.10
        openssl_target=solaris64-x86_64-gcc
        ldlibs=
        ;;
    sparc64)
        target=sparc64-sun-solaris2.10
        openssl_target=solaris64-sparcv9-gcc
        ldlibs=
        ;;
    *)
        echo >&2 "Unknown architecture: $arch"
        exit 1
        ;;
esac
readonly target
readonly openssl_target
readonly ldlibs

readonly sha256sum="e15dda82fe2fe8139dc2ac21a36d4ca01d5313c75f99f46c4e8a27709b7294bf"
readonly filename="openssl-3.4.0"
readonly tarball="$filename.tar.gz"

cd /tmp

curl -OL "https://github.com/openssl/openssl/releases/download/$filename/$tarball"
echo "$sha256sum  $tarball" > openssl.sha256sum
sha256sum --check openssl.sha256sum

tar xzf "$tarball"

prefix="/opt/cross/openssl/$target"
cd "$filename"
patch -p0 < "${BASH_SOURCE%/*}/openssl.patch"
env \
  LDLIBS="$ldlibs" \
  LDFLAGS="-Wl,-z,noexecstack" \
  ./Configure \
    --prefix="$prefix" \
    --cross-compile-prefix="/opt/cross/bin/$target-" \
    --api=1.1.1 \
    "$openssl_target" \
    no-deprecated \
    no-shared
if ! make -j $(nproc) >make.log 2>&1; then
    tail -1000 make.log
    exit 1
fi
if ! make install_sw >>make.log 2>&1; then
    tail -1000 make.log
    exit 1
fi

tar czf /root/openssl.tar.gz -C / "${prefix#/}"
