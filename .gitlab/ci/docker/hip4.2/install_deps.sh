#!/bin/sh

set -e

apt-get update

# Install development tools.
apt-get install -y --no-install-recommends \
    g++ \
    curl \
    git

apt-get clean
