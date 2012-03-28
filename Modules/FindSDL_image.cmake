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

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(SDLIMAGE_INCLUDE_DIR SDL_image.h
  HINTS
    ENV SDLIMAGEDIR
    ENV SDLDIR
  PATH_SUFFIXES include include/SDL
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/SDL12
  /usr/local/include/SDL11 # FreeBSD ports
  /usr/include/SDL12
  /usr/include/SDL11
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

find_library(SDLIMAGE_LIBRARY
  NAMES SDL_image
  HINTS
    ENV SDLIMAGEDIR
    ENV SDLDIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDLIMAGE
                                  REQUIRED_VARS SDLIMAGE_LIBRARY SDLIMAGE_INCLUDE_DIR)
