#!/bin/sh

set -e

readonly acpp_version=25.10.0

cd
if test -e AdaptiveCpp/.git; then
    cd AdaptiveCpp
    git switch --detach v$acpp_version
else
    git clone -b v$acpp_version https://github.com/AdaptiveCpp/AdaptiveCpp.git
    cd AdaptiveCpp
fi

cmake --fresh \
    -B build \
    -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/adaptivecpp \
    -DCMAKE_C_COMPILER=/usr/lib64/llvm20/bin/clang \
    -DCMAKE_CXX_COMPILER=/usr/lib64/llvm20/bin/clang++ \
    -DLLVM_DIR=/usr/lib64/llvm20/lib64/cmake/llvm \
    -DCLANG_INCLUDE_PATH=/usr/lib/clang/20 \
    -DACPP_COMPILER_FEATURE_PROFILE=full \
    -DWITH_CUDA_BACKEND=OFF \
    -DWITH_ROCM_BACKEND=OFF \
    -DWITH_OPENCL_BACKEND=OFF \
    -DWITH_LEVEL_ZERO_BACKEND=OFF \
    -DDEFAULT_TARGETS=omp

cmake --build build

DESTDIR=~/acpp-destdir cmake --install build

tar -C ~/acpp-destdir -cf ~/acpp.tar opt
