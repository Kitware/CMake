#!/bin/sh
#
# CMake UNIX Release Configuration Script.
#
# This is sourced by the cmake_release_unix_build and
# cmake_release_unix_package to setup the configuration.
#

# Check the command line arguments.
CONFIG_FILE=$1
if test "x$1" = "x"; then
  echo "Usage:"
  echo "  $0 <configuration-file>"
  echo ""
  echo "Sample configuration file:"
  echo "  RELEASE_TAG=\"Release-1-4\""
  echo "  VERSION=\"1.4\""
  echo "  PLATFORM=\"x86-linux\""
  exit 1
fi

# Set some defaults here.  They can be changed by the configuration
# file.
CVSROOT=":pserver:anonymous@www.cmake.org:/cvsroot/CMake"
CAT="cat"
COMPRESS="compress"
CREATE_SOURCE_TARBALL="no"
CVS="cvs"
FIND="find"
GZIP="gzip"
MAKE="make"
MKDIR="mkdir"
RELEASE_ROOT=`pwd`
STRIP="strip"
TAR="tar"
TOUCH="touch"
CC="gcc"
CXX="c++"
CFLAGS=""
CXXFLAGS=""
PREFIX="/usr/local"
INSTALL_SUBDIRS="bin share doc"
DOC_DIR="/doc/cmake"
# Functions can be replaced by configuration file.

# Create extra files in the installation tree.  This allows
# configurations to add documentation.
CreateExtraFiles()
{
  return 0
}

# Create a package file.  This allows configurations to create
# packages for certain UNIX distributions.
CreatePackage()
{
  return 0
}

# Write entries into the cache file before building cmake.
WriteCMakeCache()
{
${CAT} > CMakeCache.txt <<EOF
BUILD_TESTING:BOOL=OFF
EOF
}

#-----------------------------------------------------------------------------
# Configuration options.
echo "Reading configuration from ${CONFIG_FILE}..."
if . ${CONFIG_FILE} ; then : ; else
  echo "Error reading configuration."
  exit 1
fi
#-----------------------------------------------------------------------------

export CC CXX CFLAGS CXXFLAGS

# Select directories.
INSTALL_DIR="${RELEASE_ROOT}/Install"
TARBALL_DIR="${RELEASE_ROOT}/Tarballs"
SOURCE_DIR="${RELEASE_ROOT}/CMake-$VERSION"
BUILD_DIR="${RELEASE_ROOT}/CMake-$VERSION-$PLATFORM-build"
LOG_DIR="${RELEASE_ROOT}/Logs"
INSTALL_OPTIONS="DESTDIR=\"${INSTALL_DIR}\""

if [ -z "$SOURCE_TARBALL_NAME" ]; then
  SOURCE_TARBALL_NAME="CMake$VERSION-src-unix"
fi
