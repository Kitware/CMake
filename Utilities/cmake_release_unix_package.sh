#!/bin/sh
#
# CMake UNIX Release Script.
#
# Run this in the directory where cmake_release_unix_build was run.
#

# Find our own script's location.
SELFPATH=`cd \`echo $0 | sed -n '/\//{s/\/[^\/]*$//;p;}'\`;pwd`

# Read the configuration.
. ${SELFPATH}/cmake_release_unix_config.sh

# Cleanup from possible previous run.
rm -rf ${INSTALL_DIR} ${TARBALL_DIR}
mkdir -p ${INSTALL_DIR} ${TARBALL_DIR}

# Run the installation.
cd ${BUILD_DIR}
echo "Running make install ${INSTALL_OPTIONS}..."
if ${MAKE} install ${INSTALL_OPTIONS} > ${LOG_DIR}/make_install.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/make_install.log"
  exit 1
fi

# Strip the executables.
echo "Stripping executables..."
if ${STRIP} ${INSTALL_DIR}${PREFIX}/bin/* \
     > ${LOG_DIR}/strip.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/strip.log"
  exit 1
fi

# Make the source tarball if requested.
if test "${CREATE_SOURCE_TARBALL}" = "yes" ; then
  (
  cd ${RELEASE_ROOT}
  TARBALL="${TARBALL_DIR}/${SOURCE_TARBALL_NAME}.tar"
  echo "Creating ${SOURCE_TARBALL_NAME}.tar"
  if ${TAR} cvf $TARBALL CMake-$VERSION \
       > ${LOG_DIR}/${SOURCE_TARBALL_NAME}.log 2>&1 ; then : ; else
    echo "Error, see ${LOG_DIR}/${SOURCE_TARBALL_NAME}.log"
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
  )
fi

# Let the configuration file add some files.
cd ${BUILD_DIR}
CreateExtraFiles

# Create the manifest file.
echo "Writing MANIFEST..."
${MKDIR} -p ${INSTALL_DIR}${PREFIX}${DOC_DIR}
${TOUCH} ${INSTALL_DIR}${PREFIX}${DOC_DIR}/MANIFEST
cd ${INSTALL_DIR}${PREFIX}
FILES=`${FIND} ${INSTALL_SUBDIRS} -type f |sed 's/^\.\///'`
${CAT} >> ${INSTALL_DIR}${PREFIX}${DOC_DIR}/MANIFEST <<EOF
${FILES}
EOF

# Allow the configuration to create package files if it wants to do so.
CreatePackage

# Create the release tarballs.
INTERNAL_NAME="cmake-$VERSION-$PLATFORM-files"
echo "Creating ${INTERNAL_NAME}.tar"
cd ${INSTALL_DIR}${PREFIX}
if ${TAR} cvf ${INSTALL_DIR}/${INTERNAL_NAME}.tar ${INSTALL_SUBDIRS} \
     > ${LOG_DIR}/${INTERNAL_NAME}.log 2>&1 ; then : ; else
  echo "Error, see ${LOG_DIR}/${INTERNAL_NAME}.log"
  exit 1
fi

echo "Writing README"
cd ${INSTALL_DIR}
${CAT} >> README <<EOF
CMake $VERSION binary for $PLATFORM

Extract the file "${INTERNAL_NAME}.tar" into your destination
directory (typically /usr/local).  The following files will be
extracted:

${FILES}

EOF

TARBALL="${TARBALL_DIR}/CMake$VERSION-$PLATFORM.tar"
echo "Creating CMake$VERSION-$PLATFORM.tar"
if ${TAR} cvf $TARBALL README ${INTERNAL_NAME}.tar \
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
