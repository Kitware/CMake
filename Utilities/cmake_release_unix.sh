#!/bin/sh
#
# CMake UNIX Release Script.
#
# Run this in an empty directory.  Your ~/.cvspass file should already
# have an entry for the CVSROOT used below.
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
RELEASE_ROOT=`pwd`
CREATE_SOURCE_TARBALL="no"
CMAKE="cmake"
GZIP="gzip"
COMPRESS="compress"
TAR="tar"
CAT="cat"
MAKE="make"
FIND="find"
CVS="cvs"
CC="gcc"
CXX="c++"
CFLAGS=""
CXXFLAGS=""
CMAKE_CACHE_ENTRIES=""

#-----------------------------------------------------------------------------
# Configuration options (could be in separate file)
echo "Reading configuration from ${CONFIG_FILE}..."
if . `pwd`/${CONFIG_FILE} ; then : ; else
  echo "Error reading configuration."
  exit 1
fi
#-----------------------------------------------------------------------------

export CC CXX CFLAGS CXXFLAGS

# Select directories.
INSTALL_DIR="${RELEASE_ROOT}/Install"
TARBALL_DIR="${RELEASE_ROOT}/Tarballs"
BUILD_DIR="${RELEASE_ROOT}/CMake-$VERSION-$PLATFORM-build"
LOG_DIR="${RELEASE_ROOT}/Logs"

# Cleanup from possible previous run.
rm -rf ${LOG_DIR} ${BUILD_DIR} ${INSTALL_DIR} ${TARBALL_DIR}
mkdir -p ${LOG_DIR} ${BUILD_DIR} ${INSTALL_DIR} ${TARBALL_DIR}

# Make sure the source is exported from CVS.
SOURCE_DIR="${RELEASE_ROOT}/CMake-$VERSION"
if test ! -d ${SOURCE_DIR} ; then
  cd ${RELEASE_ROOT}
  rm -rf CMake
  echo "Exporing CMake from CVS..."
  if ${CVS} -z3 -d ${CVSROOT} export -r ${RELEASE_TAG} \
       CMake > ${LOG_DIR}/cvs.log 2>&1 ; then : ; else
    echo "Error, see ${LOG_DIR}/cvs.log"
    exit 1
  fi
  mv CMake CMake-$VERSION
fi

# Make the source tarball if requested.
if test "${CREATE_SOURCE_TARBALL}" = "yes" ; then
  TARBALL="${TARBALL_DIR}/CMake$VERSION-src-unix.tar"
  echo "Creating CMake$VERSION-src-unix.tar"
  if ${TAR} cvf $TARBALL CMake-$VERSION \
       > ${LOG_DIR}/CMake$VERSION-src-unix.log 2>&1 ; then : ; else
    "Error, see ${LOG_DIR}/CMake$VERSION-src-unix.log"
    exit 1
  fi
  if test "x${GZIP}" != "x" ; then
    echo "Creating $TARBALL.gz"
    ${GZIP} -c $TARBALL > $TARBALL.gz
  fi
  if test "x${COMPRESS}" != "x" ; then
    echo "Creating $TARBALL.Z"
    ${COMPRESS} $TARBALL
  fi
fi

# Build the release.
cd ${BUILD_DIR}
echo "Writing CMakeCache.txt..."
${CAT} > CMakeCache.txt <<EOF
BUILD_TESTING:BOOL=OFF
CMAKE_INSTALL_PREFIX:PATH=${INSTALL_DIR}
${CMAKE_CACHE_ENTRIES}
EOF

echo "Running CMake..."
if ${CMAKE} ${SOURCE_DIR} > ${LOG_DIR}/cmake.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/cmake.log"
  exit 1
fi

echo "Running make..."
if ${MAKE} > ${LOG_DIR}/make.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/make.log"
  exit 1
fi

echo "Running make install..."
if ${MAKE} install > ${LOG_DIR}/make_install.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/make_install.log"
  exit 1
fi

# Create the release tarballs.
cd ${INSTALL_DIR}
echo "Creating cmake-$VERSION-$PLATFORM.tar"

if ${TAR} cvf cmake-$VERSION-$PLATFORM.tar bin share \
     > ${LOG_DIR}/cmake-$VERSION-$PLATFORM.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/cmake-$VERSION-$PLATFORM.log"
  exit 1
fi

echo "Writing README"
FILES=`${FIND} bin share -type f |sed 's/^\.\///'`
${CAT} >> README <<EOF
CMake $VERSION binary for $PLATFORM

Extract the file "cmake-$VERSION-$PLATFORM.tar" into your
destination directory.  The following files will be extracted:

${FILES}

EOF

TARBALL="${TARBALL_DIR}/CMake$VERSION-$PLATFORM.tar"
echo "Creating CMake$VERSION-$PLATFORM.tar"
if ${TAR} cvf $TARBALL README cmake-$VERSION-$PLATFORM.tar \
     > ${LOG_DIR}/CMake$VERSION-$PLATFORM.log 2>&1 ; then : ; else
  "Error, see ${LOG_DIR}/CMake$VERSION-$PLATFORM.log"
  exit 1
fi

if test "x${GZIP}" != "x" ; then
  echo "Creating $TARBALL.gz"
  ${GZIP} -c $TARBALL > $TARBALL.gz
fi

if test "x${COMPRESS}" != "x" ; then
  echo "Creating $TARBALL.Z"
  ${COMPRESS} $TARBALL
fi
