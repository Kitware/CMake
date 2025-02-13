#!/usr/bin/env bash

set -e

arch="$1"
readonly arch

case "$arch" in
    i386)
        tarball="sysroot-i386-pc-solaris2.10-sunos5.10-1.tar.xz"
        sha256sum="1b9251699f4e412ba5b0fde9c0fb96ceef6b8a1f47f0c1f2146ba0ba9da458b8"
        ;;
    sparc)
        tarball="sysroot-sparc-sun-solaris2.10-sunos5.10-1.tar.xz"
        sha256sum="e6c668a63dc00de443d07cbe2be779335642ffe1b818ba85d23ab543982aaf23"
        ;;
    *)
        echo >&2 "Unknown architecture: $arch"
        exit 1
        ;;
esac
# To build externally, provide a Solaris sysroot tarball:
#   --build-arg SYSROOT_URL=...
#   --build-arg SYSROOT_SHA256SUM=...
# The tarball must contain one of:
#   sysroot/i386-pc-solaris2.10/{lib,usr/lib,usr/include}
#   sysroot/sparc-sun-solaris2.10/{lib,usr/lib,usr/include}
# The content may be retrieved from a real Solaris host.
if test -n "$SYSROOT_URL"; then
    url="$SYSROOT_URL"
    if test -n "$SYSROOT_SHA256SUM"; then
        sha256sum="$SYSROOT_SHA256SUM"
    else
        sha256sum=""
    fi
    tarball=$(basename "$url")
else
    # This URL is only visible inside of Kitware's network.
    url="https://cmake.org/files/dependencies/internal/sunos/$tarball"
fi
readonly url
readonly tarball
readonly sha256sum

cd /tmp

curl -OL "$url"
if test -n "$sha256sum"; then
    echo "$sha256sum  $tarball" > sysroot.sha256sum
    sha256sum --check sysroot.sha256sum
fi

tar xf "$tarball" -C /opt/cross
