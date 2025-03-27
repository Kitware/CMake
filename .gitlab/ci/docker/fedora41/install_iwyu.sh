#!/bin/sh

set -e

# Install development tools.
dnf install \
    --setopt=install_weak_deps=False \
    --setopt=fastestmirror=True \
    --setopt=max_parallel_downloads=10 \
    -y \
    $(grep '^[^#]\+$' /root/iwyu_packages.lst)

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
    "-DIWYU_RESOURCE_RELATIVE_TO=clang" \
    "-DIWYU_RESOURCE_DIR=../lib/clang/$llvm_version" \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/lib/llvm-$llvm_version" \
    ..
ninja
DESTDIR=/root/iwyu-destdir ninja install
tar -C /root/iwyu-destdir -cf /root/iwyu.tar .
