# Locate PhysFS library
# This module defines
# PHYSFS_LIBRARY, the name of the library to link against
# PHYSFS_FOUND, if false, do not try to link to PHYSFS
# PHYSFS_INCLUDE_DIR, where to find physfs.h
#
# $PHYSFSDIR is an environment variable that would
# correspond to the ./configure --prefix=$PHYSFSDIR
# used in building PHYSFS.
#
# Created by Eric Wing. 

FIND_PATH(PHYSFS_INCLUDE_DIR physfs.h
  PATHS
  $ENV{PHYSFSDIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)

FIND_PATH(PHYSFS_INCLUDE_DIR physfs.h
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  PATH_SUFFIXES include/physfs include
)

FIND_LIBRARY(PHYSFS_LIBRARY 
  NAMES physfs
  PATHS
  $ENV{PHYSFSDIR}
  NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(PHYSFS_LIBRARY 
  NAMES physfs
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
    PATH_SUFFIXES lib64 lib
)

SET(PHYSFS_FOUND "NO")
IF(PHYSFS_LIBRARY AND PHYSFS_INCLUDE_DIR)
  SET(PHYSFS_FOUND "YES")
ENDIF(PHYSFS_LIBRARY AND PHYSFS_INCLUDE_DIR)

