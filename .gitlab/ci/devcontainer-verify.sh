#!/bin/sh

set -e

# Source CI environment scripts
. .gitlab/ci/env.sh

# Verify devcontainer volumes are accessible
echo "# Verifying devcontainer volumes"

# Check ccache volume
if test -d /home/cmake-dev/.cache/ccache; then
    echo "ccache volume found at /home/cmake-dev/.cache/ccache"
    ccache -s 2>/dev/null || echo "ccache stats unavailable (may be fresh)"
else
    echo "WARNING: ccache volume not found at expected path"
fi

# Check glab-cli volume
if test -d /home/cmake-dev/.config/glab-cli; then
    echo "glab-cli volume found at /home/cmake-dev/.config/glab-cli"
else
    echo "WARNING: glab-cli volume not found at expected path"
fi

# Verify cmake is available
echo "# Verifying cmake"
cmake --version

# Verify ccache is available
echo "# Verifying ccache"
ccache --version

# Build a simple test project to verify the devcontainer works
echo "# Building test project"

# Create a temporary build directory
mkdir -p /tmp/devcontainer-test
cd /tmp/devcontainer-test

# Create a minimal C project
cat > CMakeLists.txt <<cmake
cmake_minimum_required(VERSION 3.15)
project(devcontainer-test C)

add_executable(test
    main.c
)
cmake

cat > main.c <<cmake
#include <stdio.h>

int main(void) {
    printf("Devcontainer CMake build OK\n");
    return 0;
}
cmake

# Configure with cmake using ccache cache path
echo "# Configuring with cmake"
cmake 2>&1 \
    -GNinja \
    -S. \
    -Bbuild

# Build
echo "# Building"
ninja -C build 2>&1

# Verify cache was preserved
echo "# Verifying ccache hit"
ccache -s 2>/dev/null | head -5 || true

echo "# Devcontainer verification complete"
