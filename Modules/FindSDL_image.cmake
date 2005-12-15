# - Locate SDL_image library
# This module defines
#  SDLIMAGE_LIBRARY, the library to link against
#  SDLIMAGE_FOUND, if false, do not try to link to SDL
#  SDLIMAGE_INCLUDE_DIR, where to find SDL/SDL.h
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc).

# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# SDLIMAGE_LIBRARY to override this selection.
FIND_PATH(SDLIMAGE_INCLUDE_DIR SDL_image.h
  $ENV{SDLIMAGEDIR}/include
  $ENV{SDLDIR}/include
  ~/Library/Frameworks/SDL_image.framework/Headers
  /Library/Frameworks/SDL_image.framework/Headers
  /usr/local/include/SDL
  /usr/include/SDL
  /usr/local/include/SDL12
  /usr/local/include/SDL11 # FreeBSD ports
  /usr/include/SDL12
  /usr/include/SDL11
  /usr/local/include
  /usr/include
  /sw/include/SDL # Fink
  /sw/include
  /opt/local/include/SDL # DarwinPorts
  /opt/local/include
  /opt/csw/include/SDL # Blastwave
  /opt/csw/include 
  /opt/include/SDL
  /opt/include
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${SDLIMAGE_INCLUDE_DIR} MATCHES ".framework")
  # Extract the path the framework resides in so we can use it for the -F flag
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" SDLIMAGE_FRAMEWORK_PATH_TEMP ${SDLIMAGE_INCLUDE_DIR})
  IF("${SDLIMAGE_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLIMAGE_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(SDLIMAGE_LIBRARY "-framework SDL_image" CACHE STRING "SDL_image framework for OSX")
  ELSE("${SDLIMAGE_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLIMAGE_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(SDLIMAGE_LIBRARY "-F${SDLIMAGE_FRAMEWORK_PATH_TEMP} -framework SDL_image" CACHE STRING "SDL_image framework for OSX")
  ENDIF("${SDLIMAGE_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${SDLIMAGE_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(SDLIMAGE_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

ELSE(${SDLIMAGE_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLIMAGE_LIBRARY 
    NAMES SDL_image
    PATHS
    $ENV{SDLIMAGEDIR}/lib
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${SDLIMAGE_INCLUDE_DIR} MATCHES ".framework")

SET(SDLIMAGE_FOUND "NO")
IF(SDLIMAGE_LIBRARY)
  SET(SDLIMAGE_FOUND "YES")
ENDIF(SDLIMAGE_LIBRARY)

