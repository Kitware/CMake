# - Locate PhysFS library
# This module defines
#  PHYSFS_LIBRARY, the name of the library to link with
#  PHYSFS_FOUND, if false, do not try to link to PHYSFS
#  PHYSFS_INCLUDE_DIR, where to find PHYSFS/PHYSFS.h
#
# $PHYSFSDIR is an environment variable that would
# correspond to the ./configure --prefix=$PHYSFSDIR
# used in building PHYSFS.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks.

# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# PHYSFS_LIBRARY to override this selection.

FIND_PATH(PHYSFS_INCLUDE_DIR NAMES physfs.h
  PATH_SUFFIXES physfs
  PATHS
  $ENV{PHYSFSDIR}/include
  ~/Library/Frameworks/PhysFS.framework/Headers
  /Library/Frameworks/PhysFS.framework/Headers
  )

# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${PHYSFS_INCLUDE_DIR} MATCHES ".framework")
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" PHYSFS_FRAMEWORK_PATH_TMP ${PHYSFS_INCLUDE_DIR})
  IF("${PHYSFS_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${PHYSFS_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(PHYSFS_LIBRARY "-framework PhysFS" CACHE STRING "PhysFS framework for OSX")
  ELSE("${PHYSFS_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${PHYSFS_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(PHYSFS_LIBRARY "-F${PHYSFS_FRAMEWORK_PATH_TMP} -framework PhysFS" CACHE STRING "PhysFS framework for OSX")
  ENDIF("${PHYSFS_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
    OR "${PHYSFS_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(PHYSFS_FRAMEWORK_PATH_TMP "" CACHE INTERNAL "")

ELSE(${PHYSFS_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(PHYSFS_LIBRARY 
    NAMES physfs PhysFS
    PATHS
    $ENV{PHYSFSDIR}/lib
    )

ENDIF(${PHYSFS_INCLUDE_DIR} MATCHES ".framework")

# handle the QUIETLY and REQUIRED arguments and set PHYSFS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PhysFS DEFAULT_MSG PHYSFS_LIBRARY PHYSFS_INCLUDE_DIR)
