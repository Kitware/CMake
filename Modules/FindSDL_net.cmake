# Locate SDL_net library
# This module defines
# SDLNET_LIBRARY, the name of the library to link against
# SDLNET_FOUND, if false, do not try to link against
# SDLNET_INCLUDE_DIR, where to find the headers
# SDLNET_VERSION_STRING - human-readable string containing the version of SDL_net
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

find_path(SDLNET_INCLUDE_DIR SDL_net.h
  HINTS
    ENV SDLNETDIR
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
find_library(SDLNET_LIBRARY
  NAMES SDL_net
  HINTS
    ENV SDLNETDIR
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

if(SDLNET_INCLUDE_DIR AND EXISTS "${SDLNET_INCLUDE_DIR}/SDL_net.h")
  file(STRINGS "${SDLNET_INCLUDE_DIR}/SDL_net.h" SDLNET_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDLNET_INCLUDE_DIR}/SDL_net.h" SDLNET_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDLNET_INCLUDE_DIR}/SDL_net.h" SDLNET_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDLNET_VERSION_MAJOR "${SDLNET_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDLNET_VERSION_MINOR "${SDLNET_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDLNET_VERSION_PATCH "${SDLNET_VERSION_PATCH_LINE}")
  set(SDLNET_VERSION_STRING ${SDLNET_VERSION_MAJOR}.${SDLNET_VERSION_MINOR}.${SDLNET_VERSION_PATCH})
  unset(SDLNET_VERSION_MAJOR_LINE)
  unset(SDLNET_VERSION_MINOR_LINE)
  unset(SDLNET_VERSION_PATCH_LINE)
  unset(SDLNET_VERSION_MAJOR)
  unset(SDLNET_VERSION_MINOR)
  unset(SDLNET_VERSION_PATCH)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDLNET
                                  REQUIRED_VARS SDLNET_LIBRARY SDLNET_INCLUDE_DIR
                                  VERSION_VAR SDLNET_VERSION_STRING)
