#!/bin/sh

set -e

apt-get update

# Install development tools.
apt-get install -y \
    curl

apt-get clean
