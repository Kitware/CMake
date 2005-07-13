# Locate SDL_image library
# This module defines
# SDLIMAGE_LIBRARY, the name of the library to link against
# SDLIMAGE_FOUND, if false, do not try to link to SDL
# SDLIMAGE_INCLUDE_DIR, where to find SDL/SDL.h
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
  ~/Library/Frameworks/SDL_image.framework/Headers
  /Library/Frameworks/SDL_image.framework/Headers
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
IF(${SDLIMAGE_INCLUDE_DIR} MATCHES ".framework")
  SET (SDLIMAGE_LIBRARY "-framework SDL_image" CACHE STRING "SDL_image framework for OSX")
ELSE(${SDLIMAGE_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLIMAGE_LIBRARY 
    NAMES SDL_image
    PATHS
    $ENV{SDLDIR}/lib
    /usr/lib
    /usr/local/lib
    /sw/lib
    )
ENDIF(${SDLIMAGE_INCLUDE_DIR} MATCHES ".framework")

SET(SDLIMAGE_FOUND "NO")
IF(SDLIMAGE_LIBRARY)
  SET(SDLIMAGE_FOUND "YES")
ENDIF(SDLIMAGE_LIBRARY)

