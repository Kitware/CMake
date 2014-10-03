#.rst:
# FindXerces
# ----------
#
# Find the Apache Xerces-C++ validating XML parser headers and libraries.
#
# This module reports information about the Xerces installation in
# several variables.  General variables::
#
#   Xerces_FOUND - true if the Xerces headers and libraries were found
#   Xerces_VERSION - Xerces release version
#   Xerces_INCLUDE_DIRS - the directory containing the Xerces headers
#   Xerces_LIBRARIES - Xerces libraries to be linked
#
# The following cache variables may also be set::
#
#   Xerces_INCLUDE_DIR - the directory containing the Xerces headers
#   Xerces_LIBRARY - the Xerces library

# Written by Roger Leigh <rleigh@codelibre.net>

#=============================================================================
# Copyright 2014 University of Dundee
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

function(_Xerces_GET_VERSION  version_hdr)
    file(STRINGS ${version_hdr} _contents REGEX "^[ \t]*#define XERCES_VERSION_.*")
    if(_contents)
        string(REGEX REPLACE ".*#define XERCES_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" Xerces_MAJOR "${_contents}")
        string(REGEX REPLACE ".*#define XERCES_VERSION_MINOR[ \t]+([0-9]+).*" "\\1" Xerces_MINOR "${_contents}")
        string(REGEX REPLACE ".*#define XERCES_VERSION_REVISION[ \t]+([0-9]+).*" "\\1" Xerces_PATCH "${_contents}")

        if(NOT Xerces_MAJOR MATCHES "^[0-9]+$")
            message(FATAL_ERROR "Version parsing failed for XERCES_VERSION_MAJOR!")
        endif()
        if(NOT Xerces_MINOR MATCHES "^[0-9]+$")
            message(FATAL_ERROR "Version parsing failed for XERCES_VERSION_MINOR!")
        endif()
        if(NOT Xerces_PATCH MATCHES "^[0-9]+$")
            message(FATAL_ERROR "Version parsing failed for XERCES_VERSION_REVISION!")
        endif()

        set(Xerces_VERSION "${Xerces_MAJOR}.${Xerces_MINOR}.${Xerces_PATCH}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Include file ${version_hdr} does not exist or does not contain expected version information")
    endif()
endfunction()

# Find include directory
find_path(Xerces_INCLUDE_DIR
          NAMES "xercesc/util/PlatformUtils.hpp"
          DOC "Xerces-C++ include directory")
mark_as_advanced(Xerces_INCLUDE_DIR)

# Find all Xerces libraries
find_library(Xerces_LIBRARY "xerces-c"
  DOC "Xerces-C++ libraries")
mark_as_advanced(Xerces_LIBRARY)

if(Xerces_INCLUDE_DIR)
  _Xerces_GET_VERSION("${Xerces_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Xerces
                                  FOUND_VAR Xerces_FOUND
                                  REQUIRED_VARS Xerces_LIBRARY
                                                Xerces_INCLUDE_DIR
                                                Xerces_VERSION
                                  VERSION_VAR Xerces_VERSION
                                  FAIL_MESSAGE "Failed to find Xerces")

if(Xerces_FOUND)
  set(Xerces_INCLUDE_DIRS "${Xerces_INCLUDE_DIR}")
  set(Xerces_LIBRARIES "${Xerces_LIBRARY}")
endif()
