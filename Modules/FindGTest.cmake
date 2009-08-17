# Locate the Google C++ Testing Framework.
#
# Defines the following variables:
#
#   GTEST_FOUND - Found the Google Testing framework
#   GTEST_INCLUDE_DIRS - Include directories
#   GTEST_LIBRARIES - The GTest library
#   GTEST_MAIN_LIBRARIES - The GTest library for automatic main()
#
# Accepts the following CMake/Environment variables as input:
#
#   GTEST_ROOT - The root directory of the gtest install prefix
#

# Copyright (c) 2009, Philip Lowman <philip@yhbt.com>
#
# Redistribution AND use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(GTEST_INCLUDE_DIR gtest/gtest.h
    HINTS
        $ENV{GTEST_ROOT}/include
        ${GTEST_ROOT}/include
)

function(_gtest_find_library _name _library)
   find_library(${_name} ${_library}
      HINTS
         $ENV{GTEST_ROOT}
         ${GTEST_ROOT}
      PATH_SUFFIXES lib64 lib
   )
endfunction()

_gtest_find_library(GTEST_LIBRARY gtest)
_gtest_find_library(GTEST_LIBRARY_DEBUG gtestd)
_gtest_find_library(GTEST_MAIN_LIBRARY gtest_main)
_gtest_find_library(GTEST_MAIN_LIBRARY_DEBUG gtest_maind)

mark_as_advanced(GTEST_INCLUDE_DIR)
mark_as_advanced(GTEST_LIBRARY)
mark_as_advanced(GTEST_LIBRARY_DEBUG)
mark_as_advanced(GTEST_MAIN_LIBRARY)
mark_as_advanced(GTEST_MAIN_LIBRARY_DEBUG)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTEST DEFAULT_MSG GTEST_INCLUDE_DIR GTEST_LIBRARY GTEST_MAIN_LIBRARY)

set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})

# Have *_LIBRARIES contain debug/release keywords if DEBUG library is available

if(GTEST_LIBRARY AND GTEST_LIBRARY_DEBUG)
   set(GTEST_LIBRARIES
         optimized ${GTEST_LIBRARY}
         debug ${GTEST_LIBRARY_DEBUG})
else()
   set(GTEST_LIBRARIES ${GTEST_LIBRARY})
endif()

if(GTEST_MAIN_LIBRARY AND GTEST_MAIN_LIBRARY_DEBUG)
   set(GTEST_MAIN_LIBRARIES
         optimized ${GTEST_MAIN_LIBRARY}
         debug ${GTEST_MAIN_LIBRARY_DEBUG})
else()
   set(GTEST_MAIN_LIBRARIES ${GTEST_MAIN_LIBRARY})
endif()

