#.rst:
# FindITK
# -------
#
# Find an ITK installation or build tree.

# When ITK is found, the ITKConfig.cmake file is sourced to setup the
# location and configuration of ITK.  Please read this file, or
# ITKConfig.cmake.in from the ITK source tree for the full list of
# definitions.  Of particular interest is ITK_USE_FILE, a CMake source file
# that can be included to set the include directories, library directories,
# and preprocessor macros.  In addition to the variables read from
# ITKConfig.cmake, this find module also defines
#  ITK_DIR  - The directory containing ITKConfig.cmake.
#             This is either the root of the build tree,
#             or the lib/InsightToolkit directory.
#             This is the only cache entry.
#
#  ITK_FOUND - Whether ITK was found.  If this is true,
#              ITK_DIR is okay.
#
#  USE_ITK_FILE - The full path to the UseITK.cmake file.
#                 This is provided for backward
#                 compatibility.  Use ITK_USE_FILE
#                 instead.

#=============================================================================
# Copyright 2001-2010 Kitware, Inc.
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

# Use the Config mode of the find_package() command to find ITKConfig.
# If this succeeds (possibly because ITK_DIR is already set), the
# command will have already loaded ITKConfig.cmake and set ITK_FOUND.
if(NOT ITK_FOUND)
  set(_ITK_REQUIRED "")
  if(ITK_FIND_REQUIRED)
    set(_ITK_REQUIRED REQUIRED)
  endif()
  set(_ITK_QUIET "")
  if(ITK_FIND_QUIETLY)
    set(_ITK_QUIET QUIET)
  endif()
  find_package(ITK ${_ITK_REQUIRED} ${_ITK_QUIET} NO_MODULE
    NAMES ITK InsightToolkit
    CONFIGS ITKConfig.cmake
    )
endif()

if(ITK_FOUND)
  # Set USE_ITK_FILE for backward-compatibility.
  set(USE_ITK_FILE ${ITK_USE_FILE})
endif()
