#!/bin/sh

set -e

cd
if [ -d include-what-you-use/.git ]; then
    cd include-what-you-use
    git pull
else
    git clone https://github.com/include-what-you-use/include-what-you-use.git
    cd include-what-you-use
fi

readonly llvm_full_version="$( clang --version | head -n1 | cut -d' ' -f3 )"
readonly llvm_version="$( echo "$llvm_full_version" | cut -d. -f-1 )"
git checkout "clang_$llvm_version"

mkdir -p build
cd build

cmake -GNinja \
    --fresh \
    -DCMAKE_BUILD_TYPE=Release \
    -DIWYU_RESOURCE_RELATIVE_TO=clang \
    -DIWYU_RESOURCE_DIR=../lib/clang/"$llvm_version" \
    -DCMAKE_INSTALL_PREFIX=/usr/local/lib/llvm-"$llvm_version" \
    ..

cmake --build . --parallel

DESTDIR=~/iwyu-destdir cmake --install .

tar -C ~/iwyu-destdir -cf ~/iwyu.tar .
