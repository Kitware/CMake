# - Locate the SDL_mixer library
# This module defines
#  SDLMIXER_LIBRARY, library to link against
#  SDLMIXER_FOUND, if false, do not try to link to SDL
#  SDLMIXER_INCLUDE_DIR, where to find SDL/SDL.h
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
# SDLMIXER_LIBRARY to override this selection.
FIND_PATH(SDLMIXER_INCLUDE_DIR SDL_mixer.h
  $ENV{SDLMIXERDIR}/include
  $ENV{SDLDIR}/include
  ~/Library/Frameworks/SDL_mixer.framework/Headers
  /Library/Frameworks/SDL_mixer.framework/Headers
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
IF(${SDLMIXER_INCLUDE_DIR} MATCHES ".framework")
  # Extract the path the framework resides in so we can use it for the -F flag
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" SDLMIXER_FRAMEWORK_PATH_TEMP ${SDLMIXER_INCLUDE_DIR})
  IF("${SDLMIXER_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLMIXER_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(SDLMIXER_LIBRARY "-framework SDL_mixer" CACHE STRING "SDL_mixer framework for OSX")
  ELSE("${SDLMIXER_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLMIXER_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(SDLMIXER_LIBRARY "-F${SDLMIXER_FRAMEWORK_PATH_TEMP} -framework SDL_mixer" CACHE STRING "SDL_mixer framework for OSX")
  ENDIF("${SDLMIXER_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${SDLMIXER_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(SDLMIXER_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

ELSE(${SDLMIXER_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLMIXER_LIBRARY 
    NAMES SDL_mixer
    PATHS
    $ENV{SDLMIXERDIR}/lib
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${SDLMIXER_INCLUDE_DIR} MATCHES ".framework")

SET(SDLMIXER_FOUND "NO")
IF(SDLMIXER_LIBRARY)
  SET(SDLMIXER_FOUND "YES")
ENDIF(SDLMIXER_LIBRARY)

