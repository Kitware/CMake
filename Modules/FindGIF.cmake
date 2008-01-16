# This module defines
# GIF_LIBRARIES - libraries to link to in order to use GIF
# GIF_FOUND, if false, do not try to link 
# GIF_INCLUDE_DIR, where to find the headers
#
# $GIF_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GIF_DIR

# Created by Eric Wing. 
# Modifications by Alexander Neundorf

FIND_PATH(GIF_INCLUDE_DIR gif_lib.h
  PATHS
  $ENV{GIF_DIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)



FIND_PATH(GIF_INCLUDE_DIR gif_lib.h
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw/include # Fink
  [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
  /usr/freeware/include
)

# the gif library can have many names :-/
SET(POTENTIAL_GIF_LIBS gif libgif ungif libungif giflib)

FIND_LIBRARY(GIF_LIBRARY 
  NAMES ${POTENTIAL_GIF_LIBS}
  PATHS
  $ENV{GIF_DIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES lib64 lib
)



FIND_LIBRARY(GIF_LIBRARY 
  NAMES ${POTENTIAL_GIF_LIBS}
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

# see readme.txt
SET(GIF_LIBRARIES ${GIF_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set GIF_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GIF  DEFAULT_MSG  GIF_LIBRARY  GIF_INCLUDE_DIR)

MARK_AS_ADVANCED(GIF_INCLUDE_DIR GIF_LIBRARY)
