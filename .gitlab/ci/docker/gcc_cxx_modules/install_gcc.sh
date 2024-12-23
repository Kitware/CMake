#!/bin/sh

set -e

readonly revision="10e702789eeabcc88451e34c2a5c7dccb96190a5" # master as of 21 Nov 2024
readonly tarball="git://gcc.gnu.org/git/gcc.git"

readonly workdir="$HOME/gcc"
readonly srcdir="$workdir/gcc"
readonly builddir="$workdir/build"
readonly njobs="$( nproc )"

mkdir -p "$workdir"
cd "$workdir"
git clone "$tarball" "$srcdir"
git -C "$srcdir" checkout "$revision"
mkdir -p "$builddir"
cd "$builddir"
"$srcdir/configure" \
    --disable-multilib \
    --enable-languages=c,c++ \
    --prefix="/opt/gcc-importstd"
make "-j$njobs"
make "-j$njobs" install-strip
rm -rf "$workdir"
