#!/bin/sh

set -e

# Install development tools.
apt-get install -y $(grep '^[^#]\+$' /root/iwyu_packages.lst)

cd /root
git clone "https://github.com/include-what-you-use/include-what-you-use.git"
cd include-what-you-use
readonly llvm_version="$( clang-15 --version | head -n1 | cut -d' ' -f4 | cut -d. -f-1 )"
git checkout "clang_$llvm_version"
mkdir build
cd build

cmake -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    "-DCMAKE_INSTALL_PREFIX=/usr/lib/llvm-$llvm_version" \
    ..
ninja
DESTDIR=/root/iwyu-destdir ninja install
tar -C /root/iwyu-destdir -cf /root/iwyu.tar .
