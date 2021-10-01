#!/bin/sh

set -e

apt-get update

# Update base packages.
apt-get install -y \
    libgnutls30 \
    libssl1.0.0 \
    openssl

# Install development tools.
apt-get install -y \
    g++ \
    clang-3.8 \
    curl \
    git

apt-get clean
