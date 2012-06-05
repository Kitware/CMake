# - Try to find BZip2
# Once done this will define
#
#  BZIP2_FOUND - system has BZip2
#  BZIP2_INCLUDE_DIR - the BZip2 include directory
#  BZIP2_LIBRARIES - Link these to use BZip2
#  BZIP2_NEED_PREFIX - this is set if the functions are prefixed with BZ2_
#  BZIP2_VERSION_STRING - the version of BZip2 found (since CMake 2.8.8)

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

SET(_BZIP2_PATHS PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\Bzip2;InstallPath]"
  )

FIND_PATH(BZIP2_INCLUDE_DIR bzlib.h ${_BZIP2_PATHS} PATH_SUFFIXES include)

IF (NOT BZIP2_LIBRARIES)
    FIND_LIBRARY(BZIP2_LIBRARY_RELEASE NAMES bz2 bzip2 ${_BZIP2_PATHS} PATH_SUFFIXES lib)
    FIND_LIBRARY(BZIP2_LIBRARY_DEBUG NAMES bzip2d ${_BZIP2_PATHS} PATH_SUFFIXES lib)

    INCLUDE(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
    SELECT_LIBRARY_CONFIGURATIONS(BZIP2)
ENDIF (NOT BZIP2_LIBRARIES)

IF (BZIP2_INCLUDE_DIR AND EXISTS "${BZIP2_INCLUDE_DIR}/bzlib.h")
    FILE(STRINGS "${BZIP2_INCLUDE_DIR}/bzlib.h" BZLIB_H REGEX "bzip2/libbzip2 version [0-9]+\\.[^ ]+ of [0-9]+ ")
    STRING(REGEX REPLACE ".* bzip2/libbzip2 version ([0-9]+\\.[^ ]+) of [0-9]+ .*" "\\1" BZIP2_VERSION_STRING "${BZLIB_H}")
ENDIF (BZIP2_INCLUDE_DIR AND EXISTS "${BZIP2_INCLUDE_DIR}/bzlib.h")

# handle the QUIETLY and REQUIRED arguments and set BZip2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BZip2
                                  REQUIRED_VARS BZIP2_LIBRARIES BZIP2_INCLUDE_DIR
                                  VERSION_VAR BZIP2_VERSION_STRING)

IF (BZIP2_FOUND)
   INCLUDE(CheckLibraryExists)
   CHECK_LIBRARY_EXISTS("${BZIP2_LIBRARIES}" BZ2_bzCompressInit "" BZIP2_NEED_PREFIX)
ENDIF (BZIP2_FOUND)

MARK_AS_ADVANCED(BZIP2_INCLUDE_DIR)
