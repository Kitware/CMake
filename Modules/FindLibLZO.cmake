#.rst:
# FindLibLZO
# -----------
#
# Find LibLZO
#
# Find LibLZO headers and library
#
# ::
#
#   LIBLZO_FOUND                     - True if liblzo is found.
#   LIBLZO_INCLUDE_DIRS              - Directory where liblzo headers are located.
#   LIBLZO_LIBRARIES                 - Lzo libraries to link against.
#   LIBLZO_HAS_LZO1X_DECOMPRESS_SAFE - True if lzo1x_decompress_safe() is found (required).
#   LIBLZO_HAS_LZO1X_1_COMPRESS      - True if lzo1x_1_compress() is found (required).
#   LIBLZO_VERSION_STRING            - version number as a string (ex: "5.0.3")

#=============================================================================
# Copyright 2008 Per Øyvind Karlsen <peroyvind@mandriva.org>
# Copyright 2009 Alexander Neundorf <neundorf@kde.org>
# Copyright 2009 Helio Chissini de Castro <helio@kde.org>
# Copyright 2012 Mario Bensi <mbensi@ipsquad.net>
# Copyright 2012-2014 Konstantin Isakov <ikm@zbackup.org>
# Copyright 2013 Benjamin Koch <bbbsnowball@gmail.com> (from lzma to lzo)
# Copyright 2014 Vladimir Stackov <amigo.elite@gmail.com>
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


find_path(LIBLZO_INCLUDE_DIR lzo/lzo1x.h )
find_library(LIBLZO_LIBRARY lzo2)

if(LIBLZO_INCLUDE_DIR AND EXISTS "${LIBLZO_INCLUDE_DIR}/lzo/lzoconf.h")
    file(STRINGS "${LIBLZO_INCLUDE_DIR}/lzo/lzoconf.h" LIBLZO_HEADER_CONTENTS REGEX "#define LZO_VERSION_STRING.+\"[^\"]+\"")
    string(REGEX REPLACE ".*#define LZO_VERSION_STRING.+\"([^\"]+)\".*" "\\1" LIBLZO_VERSION_STRING "${LIBLZO_HEADER_CONTENTS}")
    unset(LIBLZO_HEADER_CONTENTS)
endif()

# We're just using two functions.
if (LIBLZO_LIBRARY)
   include(CheckLibraryExists)
   set(CMAKE_REQUIRED_QUIET_SAVE ${CMAKE_REQUIRED_QUIET})
   set(CMAKE_REQUIRED_QUIET ${LibLZMA_FIND_QUIETLY})
   CHECK_LIBRARY_EXISTS(${LIBLZO_LIBRARY} lzo1x_decompress_safe "" LIBLZO_HAS_LZO1X_DECOMPRESS_SAFE)
   CHECK_LIBRARY_EXISTS(${LIBLZO_LIBRARY} lzo1x_1_compress "" LIBLZO_HAS_LZO1X_1_COMPRESS)
   set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_SAVE})
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibLZO  REQUIRED_VARS  LIBLZO_INCLUDE_DIR
                                                         LIBLZO_LIBRARY
                                                         LIBLZO_HAS_LZO1X_DECOMPRESS_SAFE
                                                         LIBLZO_HAS_LZO1X_1_COMPRESS
                                          VERSION_VAR    LIBLZO_VERSION_STRING
                                 )

if (LIBLZO_FOUND)
    set(LIBLZO_LIBRARIES ${LIBLZO_LIBRARY})
    set(LIBLZO_INCLUDE_DIRS ${LIBLZO_INCLUDE_DIR})
endif ()

mark_as_advanced( LIBLZO_INCLUDE_DIR LIBLZO_LIBRARY )
