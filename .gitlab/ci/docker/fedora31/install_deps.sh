#!/bin/sh

# Install build requirements.
dnf install -y \
    openssl-devel

# Install development tools.
dnf install -y \
    clang-tools-extra \
    gcc-c++ \
    git-core

dnf clean all
