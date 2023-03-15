#!/bin/sh

set -e

readonly revision="6d859df46e93e04bd7a4f90d9a9056763998f638" # llvmorg-16.0.0-rc2-31-g6d859df46e93
readonly tarball="https://github.com/llvm/llvm-project/archive/$revision.tar.gz"

readonly workdir="$HOME/llvm"
readonly srcdir="$workdir/llvm"
readonly builddir="$workdir/build"

mkdir -p "$workdir"
cd "$workdir"
curl -L "$tarball" > "llvm-$revision.tar.gz"
tar xf "llvm-$revision.tar.gz"
mv "llvm-project-$revision" "$srcdir"
mkdir -p "$builddir"
cd "$builddir"
cmake -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DLLVM_ENABLE_BINDINGS=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_DOCS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_RUNTIMES=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_UTILS=OFF \
    -DLLVM_TARGETS_TO_BUILD=X86 \
    -DLLVM_TOOL_CLANG_BUILD=ON \
    -DLLVM_USE_SYMLINKS=ON \
    "-DLLVM_EXTERNAL_CLANG_SOURCE_DIR=$srcdir/clang" \
    -DLLVM_PARALLEL_LINK_JOBS=1 \
    -DCLANG_BUILD_TOOLS=ON \
    "-DCMAKE_INSTALL_PREFIX=/opt/llvm-p1689" \
    "$srcdir/llvm"
ninja
ninja install/strip
rm -rf "$workdir"
