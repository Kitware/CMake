# Locate SDL library
# This module defines
# SDL_LIBRARY, the name of the library to link against
# SDL_FOUND, if false, do not try to link to SDL
# SDL_INCLUDE_DIR, where to find SDL/SDL.h
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
# l.e.galup  9-20-02



IF (UNIX)

  FIND_LIBRARY(SDL_LIBRARY SDL
    $ENV{SDLDIR}/lib
    /usr/lib
    /usr/local/lib
  )

  FIND_PATH( SDL_INCLUDE_DIR SDL/SDL.h
    $ENV{SDLDIR}/include
    /usr/include
    /usr/local/include
  )

ENDIF (UNIX)

SET( SDL_FOUND "NO" )
IF(SDL_LIBRARY)
	SET( SDL_FOUND "YES" )
ENDIF(SDL_LIBRARY)
