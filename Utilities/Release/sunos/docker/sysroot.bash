#!/usr/bin/env bash

set -e

arch="$1"
readonly arch

case "$arch" in
    x86_64)
        tarball="sysroot-x86_64-pc-solaris2.10-sunos5.10-1.tar.xz"
        sha256sum="bea632b3ae755f89a1c0e64775437a9b29001a3fc3a3c2c6247b921776059231"
        ;;
    sparc64)
        tarball="sysroot-sparc64-sun-solaris2.10-sunos5.10-1.tar.xz"
        sha256sum="fd60cc1be951ae314ff2b4246ac055c8e5b21c39b4cd41b23ebcec709451d90f"
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
#   sysroot/x86_64-pc-solaris2.10/{lib,usr/lib,usr/include}
#   sysroot/sparc64-sun-solaris2.10/{lib,usr/lib,usr/include}
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
