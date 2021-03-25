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

# Packages needed to test find modules.
apt-get install -y \
    alsa-utils \
    doxygen graphviz \
    gnutls-dev \
    libarchive-dev \
    libblas-dev \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-python-dev \
    libboost-thread-dev \
    libbz2-dev \
    libcups2-dev \
    libcurl4-gnutls-dev \
    libfontconfig1-dev \
    libfreetype6-dev \
    libgdal-dev \
    libgif-dev \
    libgl1-mesa-dev \
    libglew-dev \
    libgsl-dev \
    libgtest-dev \
    libgtk2.0-dev \
    libinput-dev \
    libjpeg-dev \
    libjsoncpp-dev \
    liblapack-dev \
    liblzma-dev \
    libopenmpi-dev openmpi-bin \
    libpng-dev \
    libpq-dev postgresql-server-dev-11 \
    libprotobuf-dev libprotobuf-c-dev libprotoc-dev protobuf-compiler \
    libsdl-dev \
    libsqlite3-dev \
    libtiff-dev \
    libuv1-dev \
    libx11-dev \
    libxalan-c-dev \
    libxerces-c-dev \
    libxml2-dev libxml2-utils \
    libxslt-dev xsltproc \
    python2 python2-dev python-numpy pypy pypy-dev \
    python3 python3-dev python3-numpy pypy3 pypy3-dev python3-venv \
    qtbase5-dev qtbase5-dev-tools \
    ruby ruby-dev \
    swig \
    unixodbc-dev

apt-get clean
