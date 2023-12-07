#!/bin/sh

set -e

cd .gitlab

git clone https://github.com/ninja-build/ninja.git ninja-src
cmake -S ninja-src -B ninja-src/build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build ninja-src/build --parallel --target ninja
mv ninja-src/build/ninja .
rm -rf ninja-src
