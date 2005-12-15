# - Locate SDL_ttf library
# This module defines
#  SDLTTF_LIBRARY, the library to link against
#  SDLTTF_FOUND, if false, do not try to link to SDL
#  SDLTTF_INCLUDE_DIR, where to find SDL/SDL.h
#   
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc).
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# SDLTTF_LIBRARY to override this selection.
FIND_PATH(SDLTTF_INCLUDE_DIR SDL_ttf.h
  $ENV{SDLTTFDIR}/include
  $ENV{SDLDIR}/include
  ~/Library/Frameworks/SDL_ttf.framework/Headers
  /Library/Frameworks/SDL_ttf.framework/Headers
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
IF(${SDLTTF_INCLUDE_DIR} MATCHES ".framework")
  # Extract the path the framework resides in so we can use it for the -F flag
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" SDLTTF_FRAMEWORK_PATH_TEMP ${SDLTTF_INCLUDE_DIR})
  IF("${SDLTTF_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLTTF_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(SDLTTF_LIBRARY "-framework SDL_ttf" CACHE STRING "SDL_ttf framework for OSX")
  ELSE("${SDLTTF_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLTTF_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(SDLTTF_LIBRARY "-F${SDLTTF_FRAMEWORK_PATH_TEMP} -framework SDL_ttf" CACHE STRING "SDL_ttf framework for OSX")
  ENDIF("${SDLTTF_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${SDLTTF_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(SDLTTF_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

ELSE(${SDLTTF_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLTTF_LIBRARY 
    NAMES SDL_ttf
    PATHS
    $ENV{SDLTTFDIR}/lib
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${SDLTTF_INCLUDE_DIR} MATCHES ".framework")

SET(SDLTTF_FOUND "NO")
IF(SDLTTF_LIBRARY)
  SET(SDLTTF_FOUND "YES")
ENDIF(SDLTTF_LIBRARY)

