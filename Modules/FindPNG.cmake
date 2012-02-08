# - Find the native PNG includes and library
#
# This module searches libpng, the library for working with PNG images.
#
# It defines the following variables
#  PNG_INCLUDE_DIRS, where to find png.h, etc.
#  PNG_LIBRARIES, the libraries to link against to use PNG.
#  PNG_DEFINITIONS - You should add_definitons(${PNG_DEFINITIONS}) before compiling code that includes png library files.
#  PNG_FOUND, If false, do not try to use PNG.
#  PNG_VERSION_STRING - the version of the PNG library found (since CMake 2.8.8)
# Also defined, but not for general use are
#  PNG_LIBRARY, where to find the PNG library.
# For backward compatiblity the variable PNG_INCLUDE_DIR is also set. It has the same value as PNG_INCLUDE_DIRS.
#
# Since PNG depends on the ZLib compression library, none of the above will be
# defined unless ZLib can be found.

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

if(PNG_FIND_QUIETLY)
  set(_FIND_ZLIB_ARG QUIET)
endif(PNG_FIND_QUIETLY)
find_package(ZLIB ${_FIND_ZLIB_ARG})

if(ZLIB_FOUND)
  find_path(PNG_PNG_INCLUDE_DIR png.h
  /usr/local/include/libpng             # OpenBSD
  )

  set(PNG_NAMES ${PNG_NAMES} png libpng png15 libpng15 png15d libpng15d png14 libpng14 png14d libpng14d png12 libpng12 png12d libpng12d)
  find_library(PNG_LIBRARY NAMES ${PNG_NAMES} )

  if (PNG_LIBRARY AND PNG_PNG_INCLUDE_DIR)
      # png.h includes zlib.h. Sigh.
      SET(PNG_INCLUDE_DIRS ${PNG_PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} )
      SET(PNG_INCLUDE_DIR ${PNG_INCLUDE_DIRS} ) # for backward compatiblity
      SET(PNG_LIBRARIES ${PNG_LIBRARY} ${ZLIB_LIBRARY})

      if (CYGWIN)
        if(BUILD_SHARED_LIBS)
           # No need to define PNG_USE_DLL here, because it's default for Cygwin.
        else(BUILD_SHARED_LIBS)
          SET (PNG_DEFINITIONS -DPNG_STATIC)
        endif(BUILD_SHARED_LIBS)
      endif (CYGWIN)

  endif (PNG_LIBRARY AND PNG_PNG_INCLUDE_DIR)

  if (PNG_PNG_INCLUDE_DIR AND EXISTS "${PNG_PNG_INCLUDE_DIR}/png.h")
      file(STRINGS "${PNG_PNG_INCLUDE_DIR}/png.h" png_version_str REGEX "^#define[ \t]+PNG_LIBPNG_VER_STRING[ \t]+\".+\"")

      string(REGEX REPLACE "^#define[ \t]+PNG_LIBPNG_VER_STRING[ \t]+\"([^\"]+)\".*" "\\1" PNG_VERSION_STRING "${png_version_str}")
      unset(png_version_str)
  endif (PNG_PNG_INCLUDE_DIR AND EXISTS "${PNG_PNG_INCLUDE_DIR}/png.h")
endif(ZLIB_FOUND)

# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(PNG
                                  REQUIRED_VARS PNG_LIBRARY PNG_PNG_INCLUDE_DIR
                                  VERSION_VAR PNG_VERSION_STRING)

mark_as_advanced(PNG_PNG_INCLUDE_DIR PNG_LIBRARY )
