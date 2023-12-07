#!/bin/sh

set -e

readonly revision="29862e21f6d656eca59284c927d0c4c0698eb99c" # master as of 21 Sep 2023
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
    --prefix="/opt/gcc-p1689"
make "-j$njobs"
make "-j$njobs" install-strip
rm -rf "$workdir"
