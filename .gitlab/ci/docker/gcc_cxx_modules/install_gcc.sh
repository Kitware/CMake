#!/bin/sh

set -e

readonly revision="p1689r5-cmake-ci-20230814" # 9fd54ccc390ab4eb3c48186b7bf15e02632cc76c
readonly tarball="https://github.com/mathstuf/gcc/archive/$revision.tar.gz"

readonly workdir="$HOME/gcc"
readonly srcdir="$workdir/gcc"
readonly builddir="$workdir/build"
readonly njobs="$( nproc )"

mkdir -p "$workdir"
cd "$workdir"
curl -L "$tarball" > "gcc-$revision.tar.gz"
tar xf "gcc-$revision.tar.gz"
mv "gcc-$revision" "$srcdir"
mkdir -p "$builddir"
cd "$builddir"
"$srcdir/configure" \
    --disable-multilib \
    --enable-languages=c,c++ \
    --prefix="/opt/gcc-p1689"
make "-j$njobs"
make "-j$njobs" install-strip
rm -rf "$workdir"
