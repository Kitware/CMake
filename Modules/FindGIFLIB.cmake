# This module defines
# GIFLIB_LIBRARY
# GIFLIB_FOUND, if false, do not try to link 
# GIFLIB_INCLUDE_DIR, where to find the headers
#
# $GIFLIB_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GIFLIB_DIR
#
# Created by Eric Wing. 

FIND_PATH(GIFLIB_INCLUDE_DIR gif_lib.h
  PATHS
  $ENV{GIFLIB_DIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)

FIND_PATH(GIFLIB_INCLUDE_DIR gif_lib.h
  PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)

FIND_PATH(GIFLIB_INCLUDE_DIR gif_lib.h
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
  [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
  /usr/freeware/include
)

FIND_LIBRARY(GIFLIB_LIBRARY 
  NAMES gif ungif libgif libungif
  PATHS
  $ENV{GIFLIB_DIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(GIFLIB_LIBRARY 
  NAMES gif ungif libgif libungif
  PATHS ${CMAKE_PREFIX_PATH} # Unofficial: We are proposing this.
  NO_DEFAULT_PATH
  PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(GIFLIB_LIBRARY 
  NAMES gif ungif libgif libungif
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]
  /usr/freeware
  PATH_SUFFIXES lib64 lib
)

SET(GIFLIB_FOUND "NO")
IF(GIFLIB_LIBRARY AND GIFLIB_INCLUDE_DIR)
  SET(GIFLIB_FOUND "YES")
ENDIF(GIFLIB_LIBRARY AND GIFLIB_INCLUDE_DIR)


