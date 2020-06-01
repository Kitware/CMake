#!/bin/sh

# Install build requirements.
dnf install --setopt=install_weak_deps=False -y \
    ncurses-devel \
    openssl-devel \
    qt5-qtbase-devel

# Install development tools.
dnf install --setopt=install_weak_deps=False -y \
    clang-tools-extra \
    gcc-c++ \
    git-core \
    make

# Install documentation tools.
dnf install --setopt=install_weak_deps=False -y \
    python3-sphinx \
    texinfo \
    qt5-qttools-devel

# Tools needed for the test suite.
dnf install --setopt=install_weak_deps=False -y \
    findutils \
    file

dnf clean all
