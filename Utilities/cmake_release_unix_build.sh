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
