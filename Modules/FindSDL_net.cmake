# - Locate the SDL_net library
# This module defines
#  SDLNET_LIBRARY, the library to link against
#  SDLNET_FOUND, if false, do not try to link against
#  SDLNET_INCLUDE_DIR, where to find the headers
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
# SDLNET_LIBRARY to override this selection.
FIND_PATH(SDLNET_INCLUDE_DIR SDL_net.h
  $ENV{SDLNETDIR}/include
  $ENV{SDLDIR}/include
  ~/Library/Frameworks/SDL_net.framework/Headers
  /Library/Frameworks/SDL_net.framework/Headers
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
IF(${SDLNET_INCLUDE_DIR} MATCHES ".framework")
  # Extract the path the framework resides in so we can use it for the -F flag
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" SDLNET_FRAMEWORK_PATH_TEMP ${SDLNET_INCLUDE_DIR})
  IF("${SDLNET_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLNET_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(SDLNET_LIBRARY "-framework SDL_net" CACHE STRING "SDL_net framework for OSX")
  ELSE("${SDLNET_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLNET_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(SDLNET_LIBRARY "-F${SDLNET_FRAMEWORK_PATH_TEMP} -framework SDL_net" CACHE STRING "SDL_net framework for OSX")
  ENDIF("${SDLNET_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${SDLNET_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(SDLNET_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

ELSE(${SDLNET_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLNET_LIBRARY 
    NAMES SDL_net
    PATHS
    $ENV{SDLNET}/lib
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${SDLNET_INCLUDE_DIR} MATCHES ".framework")

SET(SDLNET_FOUND "NO")
IF(SDLNET_LIBRARY)
  SET(SDLNET_FOUND "YES")
ENDIF(SDLNET_LIBRARY)

