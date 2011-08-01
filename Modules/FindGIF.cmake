# This module searches giflib and defines
# GIF_LIBRARIES - libraries to link to in order to use GIF
# GIF_FOUND, if false, do not try to link
# GIF_INCLUDE_DIR, where to find the headers
# GIF_VERSION, reports either version 4 or 3 (for everything before version 4)
#
# The minimum required version of giflib can be specified using the
# standard syntax, e.g. FIND_PACKAGE(GIF 4)
#
# $GIF_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GIF_DIR

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

# Created by Eric Wing.
# Modifications by Alexander Neundorf

FIND_PATH(GIF_INCLUDE_DIR gif_lib.h
  HINTS
  $ENV{GIF_DIR}
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /usr/freeware
)

# the gif library can have many names :-/
SET(POTENTIAL_GIF_LIBS gif libgif ungif libungif giflib giflib4)

FIND_LIBRARY(GIF_LIBRARY
  NAMES ${POTENTIAL_GIF_LIBS}
  HINTS
  $ENV{GIF_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /usr/freeware
)

# see readme.txt
SET(GIF_LIBRARIES ${GIF_LIBRARY})

# Very basic version detection.
# The GIF_LIB_VERSION string in gif_lib.h seems to be unreliable, since it seems
# to be always " Version 2.0, " in versions 3.x of giflib.
# In version 4 the member UserData was added to GifFileType, so we check for this
# one.
# http://giflib.sourcearchive.com/documentation/4.1.4/files.html
IF(GIF_INCLUDE_DIR)
  INCLUDE(CMakePushCheckState)
  INCLUDE(CheckStructHasMember)
  CMAKE_PUSH_CHECK_STATE()
  SET(GIF_VERSION 3)
  SET(CMAKE_REQUIRED_INCLUDES "${GIF_INCLUDE_DIR}")
  CHECK_STRUCT_HAS_MEMBER(GifFileType UserData gif_lib.h GIF_GifFileType_UserData )
  IF(GIF_GifFileType_UserData)
    SET(GIF_VERSION 4)
  ENDIF()
  CMAKE_POP_CHECK_STATE()
ENDIF()


# handle the QUIETLY and REQUIRED arguments and set GIF_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GIF  REQUIRED_VARS  GIF_LIBRARY  GIF_INCLUDE_DIR
                                       VERSION_VAR GIF_VERSION )

MARK_AS_ADVANCED(GIF_INCLUDE_DIR GIF_LIBRARY)
