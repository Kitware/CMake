#
# Find a VTK installation or build tree.
#
# When VTK is found, the VTKConfig.cmake file is sourced to setup the
# location and configuration of VTK.  Please read this file, or
# VTKConfig.cmake.in from the VTK source tree for the full list of
# definitions.  Of particular interest is
#
# VTK_USE_FILE          - A CMake source file that can be included
#                         to set the include directories, library
#                         directories, and preprocessor macros.
#
# In addition to the variables read from VTKConfig.cmake, this find
# module also defines
#
# VTK_DIR      - The directory containing VTKConfig.cmake.  This is either
#                the root of the build tree, or the lib/vtk
#                directory.  This is the only cache entry.
#
# VTK_FOUND    - Whether VTK was found.  If this is true, VTK_DIR is okay.
#
# USE_VTK_FILE - The full path to the UseVTK.cmake file.  This is provided
#                for backward compatability.  Use VTK_USE_FILE instead.
#

# Construct consitent error messages for use below.
SET(VTK_DIR_DESCRIPTION "directory containing VTKConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/vtk for an installation.")
SET(VTK_DIR_MESSAGE "VTK not found.  Set VTK_DIR to the ${VTK_DIR_DESCRIPTION}")

# Search only if the location is not already known.
IF(NOT VTK_DIR)
  # Get the system search path as a list.
  IF(UNIX)
    STRING(REGEX MATCHALL "[^:]+" VTK_DIR_SEARCH1 $ENV{PATH})
  ELSE(UNIX)
    STRING(REGEX REPLACE "\\\\" "/" VTK_DIR_SEARCH1 $ENV{PATH})
  ENDIF(UNIX)
  STRING(REGEX REPLACE "/;" ";" VTK_DIR_SEARCH2 ${VTK_DIR_SEARCH1})

  # Construct a set of paths relative to the system search path.
  SET(VTK_DIR_SEARCH "")
  FOREACH(dir ${VTK_DIR_SEARCH2})
    SET(VTK_DIR_SEARCH ${VTK_DIR_SEARCH} "${dir}/../lib/vtk")
  ENDFOREACH(dir)

  # Old scripts may set these directories in the CMakeCache.txt file.
  # They can tell us where to find VTKConfig.cmake.
  SET(VTK_DIR_SEARCH_LEGACY "")
  IF(VTK_BINARY_PATH)
    SET(VTK_DIR_SEARCH_LEGACY ${VTK_DIR_SEARCH_LEGACY} ${VTK_BINARY_PATH})
  ENDIF(VTK_BINARY_PATH)
  IF(VTK_INSTALL_PATH)
    SET(VTK_DIR_SEARCH_LEGACY ${VTK_DIR_SEARCH_LEGACY}
        ${VTK_INSTALL_PATH}/lib/vtk)
  ENDIF(VTK_INSTALL_PATH)

  #
  # Look for an installation or build tree.
  #
  FIND_PATH(VTK_DIR VTKConfig.cmake
    # Support legacy cache files.
    ${VTK_DIR_SEARCH_LEGACY}

    # Look in places relative to the system executable search path.
    ${VTK_DIR_SEARCH}

    # Look in standard UNIX install locations.
    /usr/local/lib/vtk
    /usr/lib/vtk

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
ENDIF(NOT VTK_DIR)

# If VTK was found, load the configuration file to get the rest of the
# settings.
IF(VTK_DIR)
  # Make sure the VTKConfig.cmake file exists in the directory provided.
  IF(EXISTS ${VTK_DIR}/VTKConfig.cmake)

    # We found VTK.  Load the settings.
    SET(VTK_FOUND 1)
    INCLUDE(${VTK_DIR}/VTKConfig.cmake)

    # Set USE_VTK_FILE for backward-compatability.
    SET(USE_VTK_FILE ${VTK_USE_FILE})
  ELSE(EXISTS ${VTK_DIR}/VTKConfig.cmake)
    # We did not find VTK.
    SET(VTK_FOUND 0)
  ENDIF(EXISTS ${VTK_DIR}/VTKConfig.cmake)
ELSE(VTK_DIR)
  # We did not find VTK.
  SET(VTK_FOUND 0)
ENDIF(VTK_DIR)

# If it was not found, explain to the user how to specify its
# location.
IF (NOT VTK_FOUND)
  IF (NOT VTK_FIND_QUIETLY)
    MESSAGE(${VTK_DIR_MESSAGE})
  ENDIF (NOT VTK_FIND_QUIETLY)
ENDIF (NOT VTK_FOUND)
