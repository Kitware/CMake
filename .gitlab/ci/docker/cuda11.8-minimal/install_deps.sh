#!/bin/sh

set -e

apt-get update

# Install dependency without interaction.
env DEBIAN_FRONTEND=noninteractive \
    TZ=America/New_York \
  apt-get install -y \
    tzdata

# Install development tools.
apt-get install -y \
    g++ \
    curl \
    git

# Reduce to minimal subset of libraries by removing static libraries
mkdir /tmp/cuda_required
mv /usr/local/cuda/lib64/libcuda* /tmp/cuda_required/
rm -f /usr/local/cuda/lib64/*static.a
mv /tmp/cuda_required/libcuda* /usr/local/cuda/lib64/
rmdir /tmp/cuda_required

apt-get clean
