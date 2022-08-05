#!/bin/sh

set -e

git clone https://github.com/ninja-build/ninja.git
cd ninja
git checkout "${1-v1.11.0}"
./configure.py --bootstrap
./ninja all
./ninja_test
strip ninja
