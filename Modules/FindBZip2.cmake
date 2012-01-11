# - Try to find BZip2
# Once done this will define
#
#  BZIP2_FOUND - system has BZip2
#  BZIP2_INCLUDE_DIR - the BZip2 include directory
#  BZIP2_LIBRARIES - Link these to use BZip2
#  BZIP2_NEED_PREFIX - this is set if the functions are prefixed with BZ2_
#  BZIP2_VERSION_STRING - the version of BZip2 found (x.y.z)

#=============================================================================
# Copyright 2006-2012 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
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

FIND_PATH(BZIP2_INCLUDE_DIR bzlib.h )

IF (NOT BZIP2_LIBRARIES)
    FIND_LIBRARY(BZIP2_LIBRARIES_RELEASE NAMES bz2 bzip2 )
    FIND_LIBRARY(BZIP2_LIBRARIES_DEBUG NAMES bzip2d bz2 bzip2 )

    IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET(BZIP2_LIBRARIES optimized "${BZIP2_LIBRARIES_RELEASE}" debug "${BZIP2_LIBRARIES_DEBUG}")
    ELSE (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET(BZIP2_LIBRARIES "${BZIP2_LIBRARIES_RELEASE}")
    ENDIF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)

    IF (BZIP2_INCLUDE_DIR AND EXISTS "${BZIP2_INCLUDE_DIR}/bzlib.h")
        FILE(STRINGS "${BZIP2_INCLUDE_DIR}/bzlib.h" BZLIB_H REGEX "bzip2/libbzip2 version [0-9]+\\.[^ ]+ of [0-9]+ ")
        STRING(REGEX REPLACE ".* bzip2/libbzip2 version ([0-9]+\\.[^ ]+) of [0-9]+ .*" "\\1" BZIP2_VERSION_STRING "${BZLIB_H}")
    ENDIF (BZIP2_INCLUDE_DIR AND EXISTS "${BZIP2_INCLUDE_DIR}/bzlib.h")
ENDIF (NOT BZIP2_LIBRARIES)

# handle the QUIETLY and REQUIRED arguments and set BZip2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BZip2
                                  REQUIRED_VARS BZIP2_LIBRARIES BZIP2_INCLUDE_DIR
                                  VERSION_VAR BZIP2_VERSION_STRING)

IF (BZIP2_FOUND)
   INCLUDE(CheckLibraryExists)
   # Make sure there is always a library to do the compile test.
   # If the user chooses a build configuration without a compatible library
   # this is a different problem.
   IF (BZIP2_LIBRARIES_DEBUG AND NOT BZIP2_LIBRARIES_RELEASE)
       CHECK_LIBRARY_EXISTS("${BZIP2_LIBRARIES_DEBUG}" BZ2_bzCompressInit "" BZIP2_NEED_PREFIX)
   ELSE (BZIP2_LIBRARIES_DEBUG AND NOT BZIP2_LIBRARIES_RELEASE)
       CHECK_LIBRARY_EXISTS("${BZIP2_LIBRARIES}" BZ2_bzCompressInit "" BZIP2_NEED_PREFIX)
   ENDIF (BZIP2_LIBRARIES_DEBUG AND NOT BZIP2_LIBRARIES_RELEASE)
ENDIF (BZIP2_FOUND)

MARK_AS_ADVANCED(BZIP2_INCLUDE_DIR BZIP2_LIBRARIES BZIP2_LIBRARIES_DEBUG BZIP2_LIBRARIES_RELEASE)
