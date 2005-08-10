# Locate SDL_mixer library
# This module defines
# SDLMIXER_LIBRARY, the name of the library to link against
# SDLMIXER_FOUND, if false, do not try to link to SDL
# SDLMIXER_INCLUDE_DIR, where to find SDL/SDL.h
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
  ~/Library/Frameworks/SDL_mixer.framework/Headers
  /Library/Frameworks/SDL_mixer.framework/Headers
  $ENV{SDLDIR}/include
  $ENV{SDLMIXERDIR}/include
  /usr/include/SDL
  /usr/include/SDL12
  /usr/include/SDL11
  /usr/include
  /usr/local/include/SDL
  /usr/local/include/SDL12
  /usr/local/include/SDL11
  /usr/local/include
  /sw/include
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${SDLMIXER_INCLUDE_DIR} MATCHES ".framework")
  SET (SDLMIXER_LIBRARY "-framework SDL_mixer" CACHE STRING "SDL_mixer framework for OSX")
ELSE(${SDLMIXER_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLMIXER_LIBRARY 
    NAMES SDL_mixer
    PATHS
    $ENV{SDLDIR}/lib
    $ENV{SDLMIXERDIR}/lib
    /usr/lib
    /usr/local/lib
    /sw/lib
    )
ENDIF(${SDLMIXER_INCLUDE_DIR} MATCHES ".framework")

SET(SDLMIXER_FOUND "NO")
IF(SDLMIXER_LIBRARY)
  SET(SDLMIXER_FOUND "YES")
ENDIF(SDLMIXER_LIBRARY)

