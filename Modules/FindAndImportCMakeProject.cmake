# FIND_AND_IMPORT_CMAKE_PROJECT can be used by projects to find other
# CMake projects.
#
# For example, the author of library FOO would include a file called
# FOOConfig.cmake in the project's distribution.  Usually such a file
# would be configured and installed by FOO's CMake code into a
# directory such as PREFIX/lib/foo.
#
# The author of application BAR that uses FOO would include the
# following lines in BAR's CMake code:
#
#  INCLUDE("${CMAKE_ROOT}/Modules/FindAndImportCMakeProject.cmake")
#  FIND_AND_IMPORT_CMAKE_PROJECT(FOO_DIR FOOConfig.cmake lib/foo)
#
# When CMake is run to build BAR, FOO_DIR will be the only
# CMakeCache.txt entry that is needed for FOO.  When FOO_DIR has been
# set correctly, FOOConfig.cmake will automatically be included to
# provide whatever settings are needed to use FOO.
#

MACRO(FIND_AND_IMPORT_CMAKE_PROJECT dir_var config_file rel_path)
  IF(NOT ${dir_var})
    # Get the system search path as a list.
    IF(UNIX)
      STRING(REGEX MATCHALL "[^:]+" ${dir_var}_SEARCH1 "$ENV{PATH}")
    ELSE(UNIX)
      STRING(REGEX REPLACE "\\\\" "/" ${dir_var}_SEARCH1 "$ENV{PATH}")
    ENDIF(UNIX)
    STRING(REGEX REPLACE "/;" ";" ${dir_var}_SEARCH2 "${${dir_var}_SEARCH1}")

    # Construct a set of paths relative to the system search path.
    SET(${dir_var}_SEARCH ${${dir_var}_SEARCH_PATH})
    FOREACH(dir ${${dir_var}_SEARCH2})
      SET(${dir_var}_SEARCH ${${dir_var}_SEARCH} "${dir}/../${rel_path}")
    ENDFOREACH(dir)

    # Look for the configuration file.
    FIND_PATH(${dir_var} ${config_file}
      # Search user paths and relative to system search path.
      ${${dir_var}_SEARCH}

      # Look in standard UNIX install locations.
      "/usr/local/${rel_path}"
      "/usr/${rel_path}"

      # Read from the CMakeSetup registry entries.  It is likely that
      # the project will have been recently built.
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]"
      "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]"

      # Help the user find it if we cannot.
      DOC "The directory containing ${config_file}."
    )
  ENDIF(NOT ${dir_var})

  # If the configuration file was found, load it.
  IF(${dir_var})
    IF(EXISTS "${${dir_var}}/${config_file}")
      # We found the configuration file.  Load the settings from it.
      INCLUDE("${${dir_var}}/${config_file}")
    ELSE(EXISTS "${${dir_var}}/${config_file}")
      IF(NOT ${dir_var}_NO_COMPLAIN)
        MESSAGE(SEND_ERROR "${dir_var} has been set to a directory "
                           "that does not contain ${config_file}.")
      ENDIF(NOT ${dir_var}_NO_COMPLAIN)
    ENDIF(EXISTS "${${dir_var}}/${config_file}")
  ENDIF(${dir_var})
ENDMACRO(FIND_AND_IMPORT_CMAKE_PROJECT)
