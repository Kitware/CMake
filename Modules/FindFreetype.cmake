# - Locate FreeType library
# This module defines
#  FREETYPE_LIBRARIES, the library to link against
#  FREETYPE_FOUND, if false, do not try to link to FREETYPE
#  FREETYPE_INCLUDE_DIRS, where to find headers.
#  FREETYPE_VERSION_STRING, the version of freetype found (since CMake 2.8.8)
#  This is the concatenation of the paths:
#  FREETYPE_INCLUDE_DIR_ft2build
#  FREETYPE_INCLUDE_DIR_freetype2
#
# $FREETYPE_DIR is an environment variable that would
# correspond to the ./configure --prefix=$FREETYPE_DIR
# used in building FREETYPE.

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
# Modifications by Alexander Neundorf.
# This file has been renamed to "FindFreetype.cmake" instead of the correct
# "FindFreeType.cmake" in order to be compatible with the one from KDE4, Alex.

# Ugh, FreeType seems to use some #include trickery which
# makes this harder than it should be. It looks like they
# put ft2build.h in a common/easier-to-find location which
# then contains a #include to a more specific header in a
# more specific location (#include <freetype/config/ftheader.h>).
# Then from there, they need to set a bunch of #define's
# so you can do something like:
# #include FT_FREETYPE_H
# Unfortunately, using CMake's mechanisms like INCLUDE_DIRECTORIES()
# wants explicit full paths and this trickery doesn't work too well.
# I'm going to attempt to cut out the middleman and hope
# everything still works.
FIND_PATH(FREETYPE_INCLUDE_DIR_ft2build ft2build.h
  HINTS
  $ENV{FREETYPE_DIR}
  PATHS
  /usr/local/X11R6/include
  /usr/local/X11/include
  /usr/freeware/include
)

FIND_PATH(FREETYPE_INCLUDE_DIR_freetype2 freetype/config/ftheader.h
  HINTS
  $ENV{FREETYPE_DIR}/include/freetype2
  PATHS
  /usr/local/X11R6/include
  /usr/local/X11/include
  /usr/freeware/include
  PATH_SUFFIXES freetype2
)

FIND_LIBRARY(FREETYPE_LIBRARY
  NAMES freetype libfreetype freetype219
  HINTS
  $ENV{FREETYPE_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  /usr/local/X11R6
  /usr/local/X11
  /usr/freeware
)

# set the user variables
IF(FREETYPE_INCLUDE_DIR_ft2build AND FREETYPE_INCLUDE_DIR_freetype2)
  SET(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_ft2build};${FREETYPE_INCLUDE_DIR_freetype2}")
ENDIF(FREETYPE_INCLUDE_DIR_ft2build AND FREETYPE_INCLUDE_DIR_freetype2)
SET(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")

IF(FREETYPE_INCLUDE_DIR_freetype2 AND EXISTS "${FREETYPE_INCLUDE_DIR_freetype2}/freetype/freetype.h")
    FILE(STRINGS "${FREETYPE_INCLUDE_DIR_freetype2}/freetype/freetype.h" freetype_version_str
         REGEX "^#[\t ]*define[\t ]+FREETYPE_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

    UNSET(FREETYPE_VERSION_STRING)
    FOREACH(VPART MAJOR MINOR PATCH)
        FOREACH(VLINE ${freetype_version_str})
            IF(VLINE MATCHES "^#[\t ]*define[\t ]+FREETYPE_${VPART}")
                STRING(REGEX REPLACE "^#[\t ]*define[\t ]+FREETYPE_${VPART}[\t ]+([0-9]+)$" "\\1"
                       FREETYPE_VERSION_PART "${VLINE}")
                IF(FREETYPE_VERSION_STRING)
                    SET(FREETYPE_VERSION_STRING "${FREETYPE_VERSION_STRING}.${FREETYPE_VERSION_PART}")
                ELSE(FREETYPE_VERSION_STRING)
                    SET(FREETYPE_VERSION_STRING "${FREETYPE_VERSION_PART}")
                ENDIF(FREETYPE_VERSION_STRING)
                UNSET(FREETYPE_VERSION_PART)
            ENDIF()
        ENDFOREACH(VLINE)
    ENDFOREACH(VPART)
ENDIF(FREETYPE_INCLUDE_DIR_freetype2 AND EXISTS "${FREETYPE_INCLUDE_DIR_freetype2}/freetype/freetype.h")


# handle the QUIETLY and REQUIRED arguments and set FREETYPE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype
                                  REQUIRED_VARS FREETYPE_LIBRARY FREETYPE_INCLUDE_DIRS
                                  VERSION_VAR FREETYPE_VERSION_STRING)

MARK_AS_ADVANCED(FREETYPE_LIBRARY FREETYPE_INCLUDE_DIR_freetype2 FREETYPE_INCLUDE_DIR_ft2build)
