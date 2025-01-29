#!/bin/sh

set -e

cd /root
git clone "https://github.com/include-what-you-use/include-what-you-use.git"
cd include-what-you-use
readonly llvm_full_version="$( clang --version | head -n1 | cut -d' ' -f3 )"
readonly llvm_version="$( echo "$llvm_full_version" | cut -d. -f-1 )"
git checkout "clang_$llvm_version"
mkdir build
cd build

cmake -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DIWYU_RESOURCE_RELATIVE_TO=clang \
    -DIWYU_RESOURCE_DIR=../lib/clang/"$llvm_version" \
    -DCMAKE_INSTALL_PREFIX=/usr/local/lib/llvm-"$llvm_version" \
    ..

cmake --build . --parallel

DESTDIR=~/iwyu-destdir cmake --install .

tar -C /root/iwyu-destdir -cf /root/iwyu.tar .
