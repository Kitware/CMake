#.rst:
# FindVTK
# -------
#
# Find a VTK installation or build tree.
#
# The following variables are set if VTK is found.  If VTK is not found,
# VTK_FOUND is set to false.
#
# ::
#
#   VTK_FOUND         - Set to true when VTK is found.
#
# The following cache entries must be set by the user to locate VTK:
#
# ::
#
#   VTK_DIR  - The directory containing VTKConfig.cmake.
#              This is either the root of the build tree,
#              or the lib/vtk directory.  This is the
#              only cache entry.
#
# The following variables are set for backward compatibility and should
# not be used in new code:
#
# ::
#
#   USE_VTK_FILE - The full path to the UseVTK.cmake file.
#                  This is provided for backward
#                  compatibility.  Use VTK_USE_FILE
#                  instead.

#=============================================================================
# Copyright 2001-2014 Kitware, Inc.
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

# Use the Config mode of the find_package() command to find VTKConfig.
# If this succeeds (possibly because VTK_DIR is already set), the
# command will have already loaded VTKConfig.cmake and set VTK_FOUND.
if(NOT VTK_FOUND)
  set(_VTK_REQUIRED "")
  if(VTK_FIND_REQUIRED)
    set(_VTK_REQUIRED REQUIRED)
  endif()
  set(_VTK_QUIET "")
  if(VTK_FIND_QUIETLY)
    set(_VTK_QUIET QUIET)
  endif()
  find_package(VTK ${_VTK_REQUIRED} ${_VTK_QUIET} NO_MODULE)
  unset(_VTK_REQUIRED)
  unset(_VTK_QUIET)
endif()

if(VTK_FOUND)
  # Set USE_VTK_FILE for backward-compatibility.
  set(USE_VTK_FILE ${VTK_USE_FILE})
endif()
