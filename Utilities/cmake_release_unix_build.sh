#!/bin/sh
#
# CMake UNIX Release Script.
#
# Run this in an empty directory.  Your ~/.cvspass file should already
# have an entry for the CVSROOT used below.
#

# Find our own script's location.
SELFPATH=`cd \`echo $0 | sed -n '/\//{s/\/[^\/]*$//;p;}'\`;pwd`

# Read the configuration.
. ${SELFPATH}/cmake_release_unix_config.sh

# Cleanup from possible previous run.
rm -rf ${LOG_DIR} ${BUILD_DIR} ${INSTALL_DIR} ${TARBALL_DIR}
mkdir -p ${LOG_DIR} ${BUILD_DIR} ${INSTALL_DIR} ${TARBALL_DIR}

# Make sure the source is exported from CVS.
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
WriteCMakeCache

echo "Running configure..."
if ${SOURCE_DIR}/configure --prefix=${PREFIX} > ${LOG_DIR}/configure.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/configure.log"
  exit 1
fi

echo "Running make..."
if ${MAKE} > ${LOG_DIR}/make.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/make.log"
  exit 1
fi
