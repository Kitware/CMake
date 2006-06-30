# - Find a VTK installation or build tree.
# The following variables are set if VTK is found.  If VTK is not
# found, VTK_FOUND is set to false.
#  VTK_FOUND         - Set to true when VTK is found.
#  VTK_USE_FILE      - CMake file to use VTK.
#  VTK_MAJOR_VERSION - The VTK major version number.
#  VTK_MINOR_VERSION - The VTK minor version number 
#                       (odd non-release).
#  VTK_BUILD_VERSION - The VTK patch level 
#                       (meaningless for odd minor).
#  VTK_INCLUDE_DIRS  - Include directories for VTK
#  VTK_LIBRARY_DIRS  - Link directories for VTK libraries
#  VTK_KITS          - List of VTK kits, in CAPS 
#                      (COMMON,IO,) etc.
#  VTK_LANGUAGES     - List of wrapped languages, in CAPS
#                      (TCL, PYHTON,) etc.
# The following cache entries must be set by the user to locate VTK:
#  VTK_DIR  - The directory containing VTKConfig.cmake.  
#             This is either the root of the build tree,
#             or the lib/vtk directory.  This is the 
#             only cache entry.
# The following variables are set for backward compatibility and
# should not be used in new code:
#  USE_VTK_FILE - The full path to the UseVTK.cmake file.
#                 This is provided for backward 
#                 compatibility.  Use VTK_USE_FILE 
#                 instead.
#

# Construct consitent error messages for use below.
SET(VTK_DIR_DESCRIPTION "directory containing VTKConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/vtk for an installation.  For VTK 4.0, this is the location of UseVTK.cmake.  This is either the root of the build tree or PREFIX/include/vtk for an installation.")
SET(VTK_DIR_MESSAGE "VTK not found.  Set the VTK_DIR cmake cache entry to the ${VTK_DIR_DESCRIPTION}")

# Search only if the location is not already known.
IF(NOT VTK_DIR)
  # Get the system search path as a list.
  IF(UNIX)
    STRING(REGEX MATCHALL "[^:]+" VTK_DIR_SEARCH1 "$ENV{PATH}")
  ELSE(UNIX)
    STRING(REGEX REPLACE "\\\\" "/" VTK_DIR_SEARCH1 "$ENV{PATH}")
  ENDIF(UNIX)
  STRING(REGEX REPLACE "/;" ";" VTK_DIR_SEARCH2 "${VTK_DIR_SEARCH1}")

  # Construct a set of paths relative to the system search path.
  SET(VTK_DIR_SEARCH "")
  FOREACH(dir ${VTK_DIR_SEARCH2})
    SET(VTK_DIR_SEARCH ${VTK_DIR_SEARCH}
      ${dir}/../lib/vtk-5.2
      ${dir}/../lib/vtk-5.1
      ${dir}/../lib/vtk-5.0
      ${dir}/../lib/vtk
      )
  ENDFOREACH(dir)

  # Old scripts may set these directories in the CMakeCache.txt file.
  # They can tell us where to find VTKConfig.cmake.
  SET(VTK_DIR_SEARCH_LEGACY "")
  IF(VTK_BINARY_PATH AND USE_BUILT_VTK)
    SET(VTK_DIR_SEARCH_LEGACY ${VTK_DIR_SEARCH_LEGACY} ${VTK_BINARY_PATH})
  ENDIF(VTK_BINARY_PATH AND USE_BUILT_VTK)
  IF(VTK_INSTALL_PATH AND USE_INSTALLED_VTK)
    SET(VTK_DIR_SEARCH_LEGACY ${VTK_DIR_SEARCH_LEGACY}
        ${VTK_INSTALL_PATH}/lib/vtk)
  ENDIF(VTK_INSTALL_PATH AND USE_INSTALLED_VTK)

  #
  # Look for an installation or build tree.
  #
  FIND_PATH(VTK_DIR UseVTK.cmake
    # Support legacy cache files.
    ${VTK_DIR_SEARCH_LEGACY}

    # Look for an environment variable VTK_DIR.
    $ENV{VTK_DIR}

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

  ELSE(EXISTS ${VTK_DIR}/VTKConfig.cmake)
    IF(EXISTS ${VTK_DIR}/UseVTK.cmake)
      # We found VTK 4.0 (UseVTK.cmake exists, but not VTKConfig.cmake).
      SET(VTK_FOUND 1)
      # Load settings for VTK 4.0.
      INCLUDE(UseVTKConfig40)
    ELSE(EXISTS ${VTK_DIR}/UseVTK.cmake)
      # We did not find VTK.
      SET(VTK_FOUND 0)
    ENDIF(EXISTS ${VTK_DIR}/UseVTK.cmake)
  ENDIF(EXISTS ${VTK_DIR}/VTKConfig.cmake)
ELSE(VTK_DIR)
  # We did not find VTK.
  SET(VTK_FOUND 0)
ENDIF(VTK_DIR)

#-----------------------------------------------------------------------------
IF(VTK_FOUND)
  # Set USE_VTK_FILE for backward-compatability.
  SET(USE_VTK_FILE ${VTK_USE_FILE})
ELSE(VTK_FOUND)
  # VTK not found, explain to the user how to specify its location.
  IF(NOT VTK_FIND_QUIETLY)
    MESSAGE(FATAL_ERROR ${VTK_DIR_MESSAGE})
  ELSE(NOT VTK_FIND_QUIETLY)
    IF(VTK_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR ${VTK_DIR_MESSAGE})
    ENDIF(VTK_FIND_REQUIRED)
  ENDIF(NOT VTK_FIND_QUIETLY)
ENDIF(VTK_FOUND)
