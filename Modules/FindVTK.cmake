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
#   VTK_USE_FILE      - CMake file to use VTK.
#   VTK_MAJOR_VERSION - The VTK major version number.
#   VTK_MINOR_VERSION - The VTK minor version number
#                        (odd non-release).
#   VTK_BUILD_VERSION - The VTK patch level
#                        (meaningless for odd minor).
#   VTK_INCLUDE_DIRS  - Include directories for VTK
#   VTK_LIBRARY_DIRS  - Link directories for VTK libraries
#   VTK_KITS          - List of VTK kits, in CAPS
#                       (COMMON,IO,) etc.
#   VTK_LANGUAGES     - List of wrapped languages, in CAPS
#                       (TCL, PYHTON,) etc.
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
# Copyright 2001-2009 Kitware, Inc.
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

# Assume not found.
set(VTK_FOUND 0)

# VTK 4.0 did not provide VTKConfig.cmake.
if("${VTK_FIND_VERSION}" VERSION_LESS 4.1)
  set(_VTK_40_ALLOW 1)
  if(VTK_FIND_VERSION)
    set(_VTK_40_ONLY 1)
  endif()
endif()

# Construct consistent error messages for use below.
set(VTK_DIR_DESCRIPTION "directory containing VTKConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/vtk for an installation.")
if(_VTK_40_ALLOW)
  set(VTK_DIR_DESCRIPTION "${VTK_DIR_DESCRIPTION}  For VTK 4.0, this is the location of UseVTK.cmake.  This is either the root of the build tree or PREFIX/include/vtk for an installation.")
endif()
set(VTK_DIR_MESSAGE "VTK not found.  Set the VTK_DIR cmake cache entry to the ${VTK_DIR_DESCRIPTION}")

# Check whether VTK 4.0 has already been found.
if(_VTK_40_ALLOW AND VTK_DIR)
  if(EXISTS ${VTK_DIR}/UseVTK.cmake AND NOT EXISTS ${VTK_DIR}/VTKConfig.cmake)
    set(VTK_FOUND 1)
    include(${CMAKE_CURRENT_LIST_DIR}/UseVTKConfig40.cmake) # No VTKConfig; load VTK 4.0 settings.
  endif()
endif()

# Use the Config mode of the find_package() command to find VTKConfig.
# If this succeeds (possibly because VTK_DIR is already set), the
# command will have already loaded VTKConfig.cmake and set VTK_FOUND.
if(NOT _VTK_40_ONLY AND NOT VTK_FOUND)
  find_package(VTK QUIET NO_MODULE)
endif()

# Special search for VTK 4.0.
if(_VTK_40_ALLOW AND NOT VTK_DIR)
  # Old scripts may set these directories in the CMakeCache.txt file.
  # They can tell us where to find VTKConfig.cmake.
  set(VTK_DIR_SEARCH_LEGACY "")
  if(VTK_BINARY_PATH AND USE_BUILT_VTK)
    set(VTK_DIR_SEARCH_LEGACY ${VTK_DIR_SEARCH_LEGACY} ${VTK_BINARY_PATH})
  endif()
  if(VTK_INSTALL_PATH AND USE_INSTALLED_VTK)
    set(VTK_DIR_SEARCH_LEGACY ${VTK_DIR_SEARCH_LEGACY}
        ${VTK_INSTALL_PATH}/lib/vtk)
  endif()

  # Look for UseVTK.cmake in build trees or under <prefix>/include/vtk.
  find_path(VTK_DIR
    NAMES UseVTK.cmake
    PATH_SUFFIXES vtk-4.0 vtk
    HINTS ENV VTK_DIR

    PATHS

    # Support legacy cache files.
    ${VTK_DIR_SEARCH_LEGACY}

    # Read from the CMakeSetup registry entries.  It is likely that
    # VTK will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]

    # Help the user find it if we cannot.
    DOC "The ${VTK_DIR_DESCRIPTION}"
    )

  if(VTK_DIR)
    if(EXISTS ${VTK_DIR}/UseVTK.cmake AND NOT EXISTS ${VTK_DIR}/VTKConfig.cmake)
      set(VTK_FOUND 1)
      include(${CMAKE_CURRENT_LIST_DIR}/UseVTKConfig40.cmake) # No VTKConfig; load VTK 4.0 settings.
    else()
      # We found the wrong version.  Pretend we did not find it.
      set(VTK_DIR "VTK_DIR-NOTFOUND" CACHE PATH "The ${VTK_DIR_DESCRIPTION}" FORCE)
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
if(VTK_FOUND)
  # Set USE_VTK_FILE for backward-compatibility.
  set(USE_VTK_FILE ${VTK_USE_FILE})
else()
  # VTK not found, explain to the user how to specify its location.
  if(VTK_FIND_REQUIRED)
    message(FATAL_ERROR ${VTK_DIR_MESSAGE})
  else()
    if(NOT VTK_FIND_QUIETLY)
      message(STATUS ${VTK_DIR_MESSAGE})
    endif()
  endif()
endif()
