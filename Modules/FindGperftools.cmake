#.rst:
# FindGperftools
# --------
#
# Find the native libuv includes and library.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   Gperftools_INCLUDE_DIRS  - where to find google/tcmalloc.h, etc.
#   Gperftools_EXECUTABLE    - full path of pproc.
#   Gperftools_LIBRARY_DIRS  - directory of libraries of gperftools.
#   Gperftools_FOUND         - True if gperftools found.
#
# ::
#
#
# Hints
# ^^^^^
#
# This module reads hints about search locations from variables:
#  GPERFTOOLS_ROOT           - Preferred installation prefix
#   (or Gperftools_ROOT)

#=============================================================================
# Copyright 2014-2015 OWenT.
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

set(_GPERFTOOLS_SEARCHES)

# Search GPERFTOOLS_ROOT first if it is set.
if(Gperftools_ROOT)
    set(GPERFTOOLS_ROOT ${Gperftools_ROOT})
endif()

if(GPERFTOOLS_ROOT)
  set(_GPERFTOOLS_SEARCH_ROOT PATHS ${GPERFTOOLS_ROOT} NO_DEFAULT_PATH)
  list(APPEND _GPERFTOOLS_SEARCHES _GPERFTOOLS_SEARCH_ROOT)
endif()

set(GPERFTOOLS_NAMES tcmalloc_minimal tcmalloc tcmalloc_and_profiler)

# Try each search configuration.
foreach(search ${_GPERFTOOLS_SEARCHES})
  find_path(Gperftools_INCLUDE_DIRS NAMES "google/tcmalloc.h" ${${search}} PATH_SUFFIXES include)
  find_program(Gperftools_EXECUTABLE NAMES pprof ${${search}} PATH_SUFFIXES bin)
  find_library(Gperftools_LIBRARY_DIRS NAMES ${GPERFTOOLS_NAMES} ${${search}} PATH_SUFFIXES lib)
  string(REGEX REPLACE "[/\\\\][^/\\\\]*$" "" Gperftools_LIBRARY_DIRS "${Gperftools_LIBRARY_DIRS}")
endforeach()

mark_as_advanced(Gperftools_LIBRARY_DIRS Gperftools_INCLUDE_DIRS Gperftools_EXECUTABLE )

# handle the QUIETLY and REQUIRED arguments and set Gperftools_FOUND to TRUE if
# all listed variables are TRUE
include("FindPackageHandleStandardArgs")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gperftools
  REQUIRED_VARS Gperftools_INCLUDE_DIRS Gperftools_EXECUTABLE Gperftools_LIBRARY_DIRS
  FOUND_VAR Gperftools_FOUND
)

if (Gperftools_FOUND)
    set(GPERFTOOLS_FOUND ${Gperftools_FOUND})
endif()

