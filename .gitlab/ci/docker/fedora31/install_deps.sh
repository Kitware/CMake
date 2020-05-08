#!/bin/sh

# Install build requirements.
dnf install -y \
    openssl-devel

# Install development tools.
dnf install --setopt=install_weak_deps=False -y \
    clang-tools-extra \
    gcc-c++ \
    git-core

# Install documentation tools.
dnf install --setopt=install_weak_deps=False -y \
    python3-sphinx \
    texinfo \
    qt5-qttools-devel

dnf clean all
