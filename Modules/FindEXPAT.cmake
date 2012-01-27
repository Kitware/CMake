# - Find expat
# Find the native EXPAT headers and libraries.
#
#  EXPAT_INCLUDE_DIRS - where to find expat.h, etc.
#  EXPAT_LIBRARIES    - List of libraries when using expat.
#  EXPAT_FOUND        - True if expat found.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

# Look for the header file.
FIND_PATH(EXPAT_INCLUDE_DIR NAMES expat.h)

# Look for the library.
FIND_LIBRARY(EXPAT_LIBRARY NAMES expat libexpat)

if (EXPAT_INCLUDE_DIR AND EXISTS "${EXPAT_INCLUDE_DIR}/expat.h")
    file(STRINGS "${EXPAT_INCLUDE_DIR}/expat.h" expat_version_str
         REGEX "^#[\t ]*define[\t ]+XML_(MAJOR|MINOR|MICRO)_VERSION[\t ]+[0-9]+$")

    unset(EXPAT_VERSION_STRING)
    foreach(VPART MAJOR MINOR MICRO)
        foreach(VLINE ${expat_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+XML_${VPART}_VERSION")
                string(REGEX REPLACE "^#[\t ]*define[\t ]+XML_${VPART}_VERSION[\t ]+([0-9]+)$" "\\1"
                       EXPAT_VERSION_PART "${VLINE}")
                if(EXPAT_VERSION_STRING)
                    set(EXPAT_VERSION_STRING "${EXPAT_VERSION_STRING}.${EXPAT_VERSION_PART}")
                else(EXPAT_VERSION_STRING)
                    set(EXPAT_VERSION_STRING "${EXPAT_VERSION_PART}")
                endif(EXPAT_VERSION_STRING)
            endif()
        endforeach(VLINE)
    endforeach(VPART)
endif (EXPAT_INCLUDE_DIR AND EXISTS "${EXPAT_INCLUDE_DIR}/expat.h")

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EXPAT
                                  REQUIRED_VARS EXPAT_LIBRARY EXPAT_INCLUDE_DIR
                                  VERSION_VAR EXPAT_VERSION_STRING)

# Copy the results to the output variables.
IF(EXPAT_FOUND)
  SET(EXPAT_LIBRARIES ${EXPAT_LIBRARY})
  SET(EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIR})
ENDIF(EXPAT_FOUND)

MARK_AS_ADVANCED(EXPAT_INCLUDE_DIR EXPAT_LIBRARY)
