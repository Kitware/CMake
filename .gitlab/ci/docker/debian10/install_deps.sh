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

# Install optional external build dependencies.
apt-get install -y \
    libarchive-dev \
    libbz2-dev \
    libcurl4-gnutls-dev \
    libexpat1-dev \
    libjsoncpp-dev \
    liblzma-dev \
    libncurses-dev \
    librhash-dev \
    libuv1-dev \
    libzstd-dev \
    zlib1g-dev

# Install iwyu runtime deps.
apt-get install -y \
    clang-6.0 \
    libncurses6

# Tools needed for the test suite.
apt-get install -y \
    jq

# Packages needed to test CTest.
apt-get install -y \
    bzr bzr-xmloutput \
    cvs \
    subversion \
    mercurial

# Install swift runtime deps.
apt-get install -y \
    libncurses5

# Packages needed to test find modules.
apt-get install -y \
    alsa-utils \
    doxygen graphviz \
    freeglut3-dev \
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
    libdevil-dev \
    libfontconfig1-dev \
    libfreetype6-dev \
    libgdal-dev \
    libgif-dev \
    libgl1-mesa-dev \
    libglew-dev \
    libgmock-dev \
    libgrpc++-dev libgrpc-dev \
    libgsl-dev \
    libgtest-dev \
    libgtk2.0-dev \
    libicu-dev \
    libinput-dev \
    libjpeg-dev \
    libjsoncpp-dev \
    liblapack-dev \
    liblzma-dev \
    libmagick++-dev \
    libopenal-dev \
    libopenmpi-dev openmpi-bin \
    libosp-dev \
    libpng-dev \
    libpq-dev postgresql-server-dev-11 \
    libprotobuf-dev libprotobuf-c-dev libprotoc-dev protobuf-compiler protobuf-compiler-grpc \
    libsdl-dev \
    libsqlite3-dev \
    libtiff-dev \
    libuv1-dev \
    libx11-dev \
    libxalan-c-dev \
    libxerces-c-dev \
    libxml2-dev libxml2-utils \
    libxslt-dev xsltproc \
    openjdk-11-jdk \
    python2 python2-dev python-numpy pypy pypy-dev \
    python3 python3-dev python3-numpy pypy3 pypy3-dev python3-venv \
    qtbase5-dev qtbase5-dev-tools \
    ruby ruby-dev \
    swig \
    unixodbc-dev

# CMake_TEST_FindPython_IronPython
apt-get install -y \
    libmono-system-windows-forms4.0-cil
curl -L -O https://github.com/IronLanguages/ironpython2/releases/download/ipy-2.7.10/ironpython_2.7.10.deb
echo 'e1aceec1d49ffa66e9059a52168a734999dcccc50164a60e2936649cae698f3e  ironpython_2.7.10.deb' > ironpython.sha256sum
sha256sum --check ironpython.sha256sum
dpkg -i ironpython_2.7.10.deb
rm ironpython_2.7.10.deb ironpython.sha256sum

# Perforce
curl -L -O https://www.perforce.com/downloads/perforce/r21.2/bin.linux26x86_64/helix-core-server.tgz
tar -C /usr/local/bin -xvzf helix-core-server.tgz -- p4 p4d
rm helix-core-server.tgz

apt-get clean
