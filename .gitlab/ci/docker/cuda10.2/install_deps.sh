#!/bin/sh

set -e

apt-get update

# Install development tools.
apt-get install -y \
    g++ \
    curl \
    git

apt-get clean
