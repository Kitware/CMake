# Locate SDL_net library
# This module defines
# SDLNET_LIBRARY, the name of the library to link against
# SDLNET_FOUND, if false, do not try to link against
# SDLNET_INCLUDE_DIR, where to find the headers
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
  ~/Library/Frameworks/SDL_net.framework/Headers
  /Library/Frameworks/SDL_net.framework/Headers
  $ENV{SDLDIR}/include
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
IF(${SDLNET_INCLUDE_DIR} MATCHES ".framework")
  SET (SDLNET_LIBRARY "-framework SDL_net" CACHE STRING "SDL_net framework for OSX")
ELSE(${SDLNET_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLNET_LIBRARY 
    NAMES SDL_net
    PATHS
    $ENV{SDLDIR}/lib
    /usr/lib
    /usr/local/lib
    /sw/lib
    )
ENDIF(${SDLNET_INCLUDE_DIR} MATCHES ".framework")

SET(SDLNET_FOUND "NO")
IF(SDLNET_LIBRARY)
  SET(SDLNET_FOUND "YES")
ENDIF(SDLNET_LIBRARY)

