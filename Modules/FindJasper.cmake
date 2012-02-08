# - Try to find the Jasper JPEG2000 library
# Once done this will define
#
#  JASPER_FOUND - system has Jasper
#  JASPER_INCLUDE_DIR - the Jasper include directory
#  JASPER_LIBRARIES - the libraries needed to use Jasper
#  JASPER_VERSION_STRING - the version of Jasper found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

FIND_PATH(JASPER_INCLUDE_DIR jasper/jasper.h)

IF (NOT JASPER_LIBRARIES)
    FIND_PACKAGE(JPEG)

    FIND_LIBRARY(JASPER_LIBRARY_RELEASE NAMES jasper libjasper)
    FIND_LIBRARY(JASPER_LIBRARY_DEBUG NAMES jasperd)

    INCLUDE(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
    SELECT_LIBRARY_CONFIGURATIONS(JASPER)
ENDIF (NOT JASPER_LIBRARIES)

IF (JASPER_INCLUDE_DIR AND EXISTS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h")
    FILE(STRINGS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h" jasper_version_str REGEX "^#define[\t ]+JAS_VERSION[\t ]+\".*\".*")

    STRING(REGEX REPLACE "^#define[\t ]+JAS_VERSION[\t ]+\"([^\"]+)\".*" "\\1" JASPER_VERSION_STRING "${jasper_version_str}")
ENDIF (JASPER_INCLUDE_DIR AND EXISTS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h")

# handle the QUIETLY and REQUIRED arguments and set JASPER_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Jasper
                                  REQUIRED_VARS JASPER_LIBRARIES JASPER_INCLUDE_DIR JPEG_LIBRARIES
                                  VERSION_VAR JASPER_VERSION_STRING)

IF (JASPER_FOUND)
   SET(JASPER_LIBRARIES ${JASPER_LIBRARIES} ${JPEG_LIBRARIES} )
ENDIF (JASPER_FOUND)

MARK_AS_ADVANCED(JASPER_INCLUDE_DIR)
