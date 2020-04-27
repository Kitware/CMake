#!/bin/sh

set -e

apt-get update

# Install build requirements.
apt-get install -y \
    libssl-dev

# Install development tools.
apt-get install -y \
    g++ \
    curl \
    git

# Install iwyu runtime deps.
apt-get install -y \
    clang-6.0 \
    libncurses6

apt-get clean
