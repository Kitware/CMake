#.rst:
# FindLibUnwind
# -----------
#
# Find LibUnwind
#
# Find LibUnwind headers and library
#
# ::
#
#   LIBUNWIND_FOUND                     - True if libunwind is found.
#   LIBUNWIND_INCLUDE_DIRS              - Directory where libunwind headers are located.
#   LIBUNWIND_LIBRARIES                 - Unwind libraries to link against.
#   LIBUNWIND_HAS_UNW_GETCONTEXT        - True if unw_getcontext() is found (required).
#   LIBUNWIND_HAS_UNW_INIT_LOCAL        - True if unw_init_local() is found (required).
#   LIBUNWIND_VERSION_STRING            - version number as a string (ex: "5.0.3")

#=============================================================================
# Copyright 2014 ZBackup contributors
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


find_path(LIBUNWIND_INCLUDE_DIR libunwind.h )
if(NOT EXISTS "${LIBUNWIND_INCLUDE_DIR}/unwind.h")
  MESSAGE("Found libunwind.h but corresponding unwind.h is absent!")
  SET(LIBUNWIND_INCLUDE_DIR "")
endif()

find_library(LIBUNWIND_LIBRARY unwind)

if(LIBUNWIND_INCLUDE_DIR AND EXISTS "${LIBUNWIND_INCLUDE_DIR}/libunwind-common.h")
  file(STRINGS "${LIBUNWIND_INCLUDE_DIR}/libunwind-common.h" LIBUNWIND_HEADER_CONTENTS REGEX "#define UNW_VERSION_[A-Z]+\t[0-9]*")

  string(REGEX REPLACE ".*#define UNW_VERSION_MAJOR\t([0-9]*).*" "\\1" LIBUNWIND_VERSION_MAJOR "${LIBUNWIND_HEADER_CONTENTS}")
  string(REGEX REPLACE ".*#define UNW_VERSION_MINOR\t([0-9]*).*" "\\1" LIBUNWIND_VERSION_MINOR "${LIBUNWIND_HEADER_CONTENTS}")
  string(REGEX REPLACE ".*#define UNW_VERSION_EXTRA\t([0-9]*).*" "\\1" LIBUNWIND_VERSION_EXTRA "${LIBUNWIND_HEADER_CONTENTS}")

  if(LIBUNWIND_VERSION_EXTRA)
    set(LIBUNWIND_VERSION_STRING "${LIBUNWIND_VERSION_MAJOR}.${LIBUNWIND_VERSION_MINOR}.${LIBUNWIND_VERSION_EXTRA}")
  else(not LIBUNWIND_VERSION_EXTRA)
    set(LIBUNWIND_VERSION_STRING "${LIBUNWIND_VERSION_MAJOR}.${LIBUNWIND_VERSION_MINOR}")
  endif()
  unset(LIBUNWIND_HEADER_CONTENTS)
endif()

if (LIBUNWIND_LIBRARY)
  include(CheckSymbolExists)
  set(CMAKE_REQUIRED_QUIET_SAVE ${CMAKE_REQUIRED_QUIET})
  set(CMAKE_REQUIRED_QUIET ${LibUnwind_FIND_QUIETLY})
  CHECK_SYMBOL_EXISTS(unw_getcontext "${LIBUNWIND_INCLUDE_DIR}/libunwind.h" LIBUNWIND_HAS_UNW_GETCONTEXT)
  CHECK_SYMBOL_EXISTS(unw_init_local "${LIBUNWIND_INCLUDE_DIR}/libunwind.h" LIBUNWIND_HAS_UNW_INIT_LOCAL)
  set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_SAVE})
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibUnwind  REQUIRED_VARS  LIBUNWIND_INCLUDE_DIR
                                                            LIBUNWIND_LIBRARY
                                                            LIBUNWIND_HAS_UNW_GETCONTEXT
                                                            LIBUNWIND_HAS_UNW_INIT_LOCAL
                                             VERSION_VAR    LIBUNWIND_VERSION_STRING
                                 )

if (LIBUNWIND_FOUND)
  set(LIBUNWIND_LIBRARIES ${LIBUNWIND_LIBRARY})
  set(LIBUNWIND_INCLUDE_DIRS ${LIBUNWIND_INCLUDE_DIR})
endif ()

mark_as_advanced( LIBUNWIND_INCLUDE_DIR LIBUNWIND_LIBRARY )
