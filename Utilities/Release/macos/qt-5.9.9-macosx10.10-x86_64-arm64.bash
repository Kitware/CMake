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
curl -OL https://download.qt.io/archive/qt/5.9/5.9.9/single/qt-everywhere-opensource-src-5.9.9.tar.xz
shasum -a 256 qt-everywhere-opensource-src-5.9.9.tar.xz | grep -q 5ce285209290a157d7f42ec8eb22bf3f1d76f2e03a95fc0b99b553391be01642
tar xjf qt-everywhere-opensource-src-5.9.9.tar.xz
patch -p0 < "${BASH_SOURCE%/*}/qt-5.9.9.patch"

# Build the x86_64 variant.
mkdir qt-5.9.9-x86_64
cd qt-5.9.9-x86_64
../qt-everywhere-opensource-src-5.9.9/configure \
  --prefix=/ \
  -platform macx-clang \
  -device-option QMAKE_APPLE_DEVICE_ARCHS=x86_64 \
  -device-option QMAKE_MACOSX_DEPLOYMENT_TARGET=10.10 \
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
mkdir qt-5.9.9-arm64
cd qt-5.9.9-arm64
../qt-everywhere-opensource-src-5.9.9/configure \
  --prefix=/ \
  -platform macx-clang \
  -device-option QMAKE_APPLE_DEVICE_ARCHS=arm64 \
  -device-option QMAKE_MACOSX_DEPLOYMENT_TARGET=10.10 \
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
# Some executables fail to link due to architecture mismatch.
# Build what we can first.
make -j 8 -k || true
# Provide needed executables using the x86_64 variants.
cp ../qt-5.9.9-x86_64/qtbase/bin/uic qtbase/bin/uic
install_name_tool -add_rpath @executable_path/../../../qt-5.9.9-x86_64/qtbase/lib qtbase/bin/uic
cp ../qt-5.9.9-x86_64/qtbase/bin/qlalr qtbase/bin/qlalr
install_name_tool -add_rpath @executable_path/../../../qt-5.9.9-x86_64/qtbase/lib qtbase/bin/qlalr
# Some parts still fail to build, but the parts we need can finish.
make -j 8 -k || true
cd ..

# Combine the two builds into universal binaries.
makeuniversal qt-5.9.9-univ qt-5.9.9-x86_64 qt-5.9.9-arm64
cd qt-5.9.9-univ
make install -j 8 INSTALL_ROOT=/tmp/qt-5.9.9-macosx10.10-x86_64-arm64
cd ..

# Create the final tarball containing universal binaries.
tar cjf qt-5.9.9-macosx10.10-x86_64-arm64.tar.xz -C /tmp qt-5.9.9-macosx10.10-x86_64-arm64
