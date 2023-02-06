#!/bin/sh

set -e

# Install development tools.
apt-get install -y $(grep '^[^#]\+$' /root/iwyu_packages.lst)

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
tar -C /root/iwyu-destdir -cf /root/iwyu.tar .
