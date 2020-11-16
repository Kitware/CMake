#!/bin/sh

set -e

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
    file \
    which

# Packages needed to test find modules.
dnf install --setopt=install_weak_deps=False -y \
    alsa-lib-devel \
    blas-devel \
    boost-devel boost-python3-devel \
    bzip2-devel \
    cups-devel \
    doxygen \
    expat-devel \
    fontconfig-devel \
    freetype-devel \
    gdal-devel \
    gettext \
    giflib-devel \
    glew-devel \
    gmock \
    gnutls-devel \
    gsl-devel \
    gtest-devel \
    gtk2-devel \
    jsoncpp-devel \
    lapack-devel \
    libarchive-devel \
    libcurl-devel \
    libinput-devel systemd-devel \
    libjpeg-turbo-devel \
    libpng-devel \
    libpq-devel postgresql-server-devel \
    libtiff-devel \
    libuv-devel \
    libxml2-devel \
    libxslt-devel \
    openmpi-devel \
    patch \
    perl \
    protobuf-devel protobuf-c-devel protobuf-lite-devel \
    pypy2 pypy2-devel \
    pypy3 pypy3-devel \
    python2 python2-devel python2-numpy \
    python3 python3-devel python3-numpy \
    python3-jsmin python3-jsonschema \
    ruby rubygems ruby-devel \
    SDL-devel \
    sqlite-devel \
    swig \
    unixODBC-devel \
    xalan-c-devel \
    xerces-c-devel \
    xz-devel

dnf clean all
