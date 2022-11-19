#!/bin/sh

set -e

readonly revision="p1689r5-cmake-ci-20220614" # 3075e510e3d29583f8886b95aff044c0474c84a5
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
