# - Find curl
# Find the native CURL headers and libraries.
#
#  CURL_INCLUDE_DIRS   - where to find curl/curl.h, etc.
#  CURL_LIBRARIES      - List of libraries when using curl.
#  CURL_FOUND          - True if curl found.
#  CURL_VERSION_STRING - the version of curl found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

# Look for the header file.
FIND_PATH(CURL_INCLUDE_DIR NAMES curl/curl.h)
MARK_AS_ADVANCED(CURL_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(CURL_LIBRARY NAMES 
    curl
  # Windows MSVC prebuilts:
    curllib
    libcurl_imp
    curllib_static
)
MARK_AS_ADVANCED(CURL_LIBRARY)

IF(CURL_INCLUDE_DIR)
  FOREACH(_curl_version_header curlver.h curl.h)
    IF(EXISTS "${CURL_INCLUDE_DIR}/curl/${_curl_version_header}")
      FILE(STRINGS "${CURL_INCLUDE_DIR}/curl/${_curl_version_header}" curl_version_str REGEX "^#define[\t ]+LIBCURL_VERSION[\t ]+\".*\"")

      STRING(REGEX REPLACE "^#define[\t ]+LIBCURL_VERSION[\t ]+\"([^\"]*)\".*" "\\1" CURL_VERSION_STRING "${curl_version_str}")
      UNSET(curl_version_str)
      BREAK()
    ENDIF()
  ENDFOREACH(_curl_version_header)
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURL
                                  REQUIRED_VARS CURL_LIBRARY CURL_INCLUDE_DIR
                                  VERSION_VAR CURL_VERSION_STRING)

IF(CURL_FOUND)
  SET(CURL_LIBRARIES ${CURL_LIBRARY})
  SET(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
ENDIF(CURL_FOUND)
