# Locate PhysFS library
# This module defines
# PHYSFS_LIBRARY, the name of the library to link against
# PHYSFS_FOUND, if false, do not try to link to PHYSFS
# PHYSFS_INCLUDE_DIR, where to find PHYSFS/PHYSFS.h
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
FIND_PATH(PHYSFS_INCLUDE_DIR physfs.h
  ~/Library/Frameworks/PhysFS.framework/Headers
  /Library/Frameworks/PhysFS.framework/Headers
  $ENV{PHYSFSDIR}/include
  /usr/include
  /usr/include/physfs
  /usr/local/include/physfs
  /sw/include
  /sw/include/physfs
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${PHYSFS_INCLUDE_DIR} MATCHES ".framework")
  SET (PHYSFS_LIBRARY "-framework PhysFS" CACHE STRING "PhysFS framework for OSX")
ELSE(${PHYSFS_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(PHYSFS_LIBRARY 
    NAMES physfs PhysFS
    PATHS
    $ENV{PHYSFSDIR}/lib
    /usr/lib
    /usr/local/lib
    /sw/lib
    )
ENDIF(${PHYSFS_INCLUDE_DIR} MATCHES ".framework")

SET(PHYSFS_FOUND "NO")
IF(PHYSFS_LIBRARY)
  SET(PHYSFS_FOUND "YES")
ENDIF(PHYSFS_LIBRARY)

