#!/usr/bin/env bash

# Run this script on a macOS x86_64 host to generate Qt universal binaries.
#
# This script requires the 'makeuniversal' tool from:
#
#   https://github.com/fizzyade/makeuniversal
#
# Build it with an existing local Qt installation first.
#
# Set the PATH environment variable to contain the location of 'makeuniversal'.

set -e
set -x

umask 022

# Verify that 'makeuniversal' is available in the PATH.
type -p makeuniversal >/dev/null

# Download, verify, and extract sources.
curl -OL https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz
shasum -a 256 qt-everywhere-src-5.15.2.tar.xz | grep -q 3a530d1b243b5dec00bc54937455471aaa3e56849d2593edb8ded07228202240
tar xjf qt-everywhere-src-5.15.2.tar.xz

# Build the x86_64 variant.
mkdir qt-5.15.2-x86_64
cd qt-5.15.2-x86_64
../qt-everywhere-src-5.15.2/configure \
  --prefix=/ \
  -platform macx-clang \
  -device-option QMAKE_APPLE_DEVICE_ARCHS=x86_64 \
  -device-option QMAKE_MACOSX_DEPLOYMENT_TARGET=10.13 \
  -release \
  -opensource -confirm-license \
  -gui \
  -widgets \
  -no-gif \
  -no-icu \
  -no-pch \
  -no-angle \
  -no-opengl \
  -no-dbus \
  -no-harfbuzz \
  -skip declarative \
  -skip multimedia \
  -skip qtcanvas3d \
  -skip qtcharts \
  -skip qtconnectivity \
  -skip qtdeclarative \
  -skip qtgamepad \
  -skip qtlocation \
  -skip qtmultimedia \
  -skip qtnetworkauth \
  -skip qtpurchasing \
  -skip qtremoteobjects \
  -skip qtscript \
  -skip qtsensors \
  -skip qtserialbus \
  -skip qtserialport \
  -skip qtsvg \
  -skip qtwebchannel \
  -skip qtwebengine \
  -skip qtwebsockets \
  -skip qtxmlpatterns \
  -nomake examples \
  -nomake tests \
  -nomake tools
make -j 8
cd ..

# Build the arm64 variant.
mkdir qt-5.15.2-arm64
cd qt-5.15.2-arm64
../qt-everywhere-src-5.15.2/configure \
  --prefix=/ \
  -platform macx-clang \
  -device-option QMAKE_APPLE_DEVICE_ARCHS=arm64 \
  -device-option QMAKE_MACOSX_DEPLOYMENT_TARGET=10.13 \
  -release \
  -opensource -confirm-license \
  -gui \
  -widgets \
  -no-gif \
  -no-icu \
  -no-pch \
  -no-angle \
  -no-opengl \
  -no-dbus \
  -no-harfbuzz \
  -skip declarative \
  -skip multimedia \
  -skip qtcanvas3d \
  -skip qtcharts \
  -skip qtconnectivity \
  -skip qtdeclarative \
  -skip qtgamepad \
  -skip qtlocation \
  -skip qtmultimedia \
  -skip qtnetworkauth \
  -skip qtpurchasing \
  -skip qtremoteobjects \
  -skip qtscript \
  -skip qtsensors \
  -skip qtserialbus \
  -skip qtserialport \
  -skip qtsvg \
  -skip qtwebchannel \
  -skip qtwebengine \
  -skip qtwebsockets \
  -skip qtxmlpatterns \
  -nomake examples \
  -nomake tests \
  -nomake tools
make -j 8 -k
cd ..

# Combine the two builds into universal binaries.
makeuniversal qt-5.15.2-univ qt-5.15.2-x86_64 qt-5.15.2-arm64
cd qt-5.15.2-univ
make install -j 8 INSTALL_ROOT=/tmp/qt-5.15.2-macosx10.13-x86_64-arm64
cd ..

# Create the final tarball containing universal binaries.
tar cjf qt-5.15.2-macosx10.13-x86_64-arm64.tar.xz -C /tmp qt-5.15.2-macosx10.13-x86_64-arm64
