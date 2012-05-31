# - Find LibLZMA
# Find LibLZMA headers and library
#
#  LIBLZMA_FOUND             - True if liblzma is found.
#  LIBLZMA_INCLUDE_DIRS      - Directory where liblzma headers are located.
#  LIBLZMA_LIBRARIES         - Lzma libraries to link against.
#  LIBLZMA_HAS_AUTO_DECODER  - True if lzma_auto_decoder() is found (required).
#  LIBLZMA_HAS_EASY_ENCODER  - True if lzma_easy_encoder() is found (required).
#  LIBLZMA_HAS_LZMA_PRESET   - True if lzma_lzma_preset() is found (required).
#  LIBLZMA_VERSION_MAJOR     - The major version of lzma
#  LIBLZMA_VERSION_MINOR     - The minor version of lzma
#  LIBLZMA_VERSION_PATCH     - The patch version of lzma
#  LIBLZMA_VERSION_STRING    - version number as a string (ex: "5.0.3")

#=============================================================================
# Copyright 2008 Per Øyvind Karlsen <peroyvind@mandriva.org>
# Copyright 2009 Alexander Neundorf <neundorf@kde.org>
# Copyright 2009 Helio Chissini de Castro <helio@kde.org>
# Copyright 2012 Mario Bensi <mbensi@ipsquad.net>
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


FIND_PATH(LIBLZMA_INCLUDE_DIR lzma.h )
FIND_LIBRARY(LIBLZMA_LIBRARY lzma)

IF(LIBLZMA_INCLUDE_DIR AND EXISTS "${LIBLZMA_INCLUDE_DIR}/lzma/version.h")
    FILE(READ "${LIBLZMA_INCLUDE_DIR}/lzma/version.h" LIBLZMA_HEADER_CONTENTS)

    STRING(REGEX REPLACE ".*#define LZMA_VERSION_MAJOR ([0-9]+).*" "\\1" LIBLZMA_VERSION_MAJOR "${LIBLZMA_HEADER_CONTENTS}")
    STRING(REGEX REPLACE ".*#define LZMA_VERSION_MINOR ([0-9]+).*" "\\1" LIBLZMA_VERSION_MINOR "${LIBLZMA_HEADER_CONTENTS}")
    STRING(REGEX REPLACE ".*#define LZMA_VERSION_PATCH ([0-9]+).*" "\\1" LIBLZMA_VERSION_PATCH "${LIBLZMA_HEADER_CONTENTS}")

    SET(LIBLZMA_VERSION_STRING "${LIBLZMA_VERSION_MAJOR}.${LIBLZMA_VERSION_MINOR}.${LIBLZMA_VERSION_PATCH}")
ENDIF(LIBLZMA_INCLUDE_DIR AND EXISTS "${LIBLZMA_INCLUDE_DIR}/lzma/version.h")

# We're using new code known now as XZ, even library still been called LZMA
# it can be found in http://tukaani.org/xz/
# Avoid using old codebase
IF (LIBLZMA_LIBRARY)
   INCLUDE(CheckLibraryExists)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_auto_decoder "" LIBLZMA_HAS_AUTO_DECODER)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_easy_encoder "" LIBLZMA_HAS_EASY_ENCODER)
   CHECK_LIBRARY_EXISTS(${LIBLZMA_LIBRARY} lzma_lzma_preset "" LIBLZMA_HAS_LZMA_PRESET)
ENDIF (LIBLZMA_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibLZMA  REQUIRED_VARS  LIBLZMA_INCLUDE_DIR
                                                          LIBLZMA_LIBRARY
                                                          LIBLZMA_HAS_AUTO_DECODER
                                                          LIBLZMA_HAS_EASY_ENCODER
                                                          LIBLZMA_HAS_LZMA_PRESET
                                           VERSION_VAR    LIBLZMA_VERSION_STRING
                                 )

IF (LIBLZMA_FOUND)
    SET(LIBLZMA_LIBRARIES ${LIBLZMA_LIBRARY})
    SET(LIBLZMA_INCLUDE_DIRS ${LIBLZMA_INCLUDE_DIR})
ENDIF (LIBLZMA_FOUND)

MARK_AS_ADVANCED( LIBLZMA_INCLUDE_DIR LIBLZMA_LIBRARY )
