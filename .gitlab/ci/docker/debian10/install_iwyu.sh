#!/bin/sh

set -e

# Install development tools.
apt-get update
apt-get install -y \
    clang-6.0 \
    libclang-6.0-dev \
    llvm-6.0-dev \
    libz-dev \
    g++ \
    cmake \
    ninja-build \
    git

cd /root
git clone "https://github.com/include-what-you-use/include-what-you-use.git"
cd include-what-you-use
readonly llvm_version="$( clang-6.0 --version | head -n1 | cut -d' ' -f3 | cut -d. -f-2 )"
git checkout "clang_$llvm_version"
mkdir build
cd build

cmake -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    "-DCMAKE_INSTALL_PREFIX=/usr/lib/llvm-$llvm_version" \
    "-DIWYU_LLVM_ROOT_PATH=/usr/lib/llvm-$llvm_version" \
    ..
ninja
DESTDIR=/root/iwyu-destdir ninja install
tar -C /root/iwyu-destdir -cf /root/iwyu.tar.gz .
