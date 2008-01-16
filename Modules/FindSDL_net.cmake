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


FIND_PATH(SDLNET_INCLUDE_DIR SDL_net.h
  PATHS
  $ENV{SDLNETDIR}
  $ENV{SDLDIR}
  NO_DEFAULT_PATH
  PATH_SUFFIXES include
)

FIND_PATH(SDLNET_INCLUDE_DIR SDL_net.h
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
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
FIND_LIBRARY(SDLNET_LIBRARY 
  NAMES SDL_net
  PATHS
  $ENV{SDLNETDIR}
  $ENV{SDLDIR}
  NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(SDLNET_LIBRARY 
  NAMES SDL_net
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
    PATH_SUFFIXES lib64 lib
)

SET(SDLNET_FOUND "NO")
IF(SDLNET_LIBRARY AND SDLNET_INCLUDE_DIR)
  SET(SDLNET_FOUND "YES")
ENDIF(SDLNET_LIBRARY AND SDLNET_INCLUDE_DIR)

