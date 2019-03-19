cmake_minimum_required(VERSION 3.10)
project(RerunMocPlugin)
include("../AutogenCoreTest.cmake")

# Tests Q_PLUGIN_METADATA and CMAKE_AUTOMOC_DEPEND_FILTERS
# json file change detection

# Dummy executable to generate a clean target
add_executable(dummy dummy.cpp)

# Utility variables
set(timeformat "%Y.%j.%H.%M%S")
set(mocPlugSrcDir "${CMAKE_CURRENT_SOURCE_DIR}/MocPlugin")
set(mocPlugBinDir "${CMAKE_CURRENT_BINARY_DIR}/MocPlugin")

# Utility macros
macro(sleep)
  message(STATUS "Sleeping for a few seconds.")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
endmacro()

macro(rebuild buildName)
  message(STATUS "Starting build ${buildName}.")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocPlugBinDir}" RESULT_VARIABLE result)
  if (result)
    message(FATAL_ERROR "Build ${buildName} failed.")
  else()
    message(STATUS "Build ${buildName} finished.")
  endif()
endmacro()

macro(require_change PLG)
  if (pl${PLG}After VERSION_GREATER pl${PLG}Before)
    message(STATUS "As expected the file ${pl${PLG}File} changed.")
  else()
    message(SEND_ERROR
      "Unexpectedly the file ${pl${PLG}File} did not change!\nTimestamp pre: ${pl${PLG}Before}\nTimestamp aft: ${pl${PLG}After}\n")
  endif()
endmacro()

macro(require_change_not PLG)
  if (pl${PLG}After VERSION_GREATER pl${PLG}Before)
    message(SEND_ERROR
      "Unexpectedly the file ${pl${PLG}File} changed!\nTimestamp pre: ${pl${PLG}Before}\nTimestamp aft: ${pl${PLG}After}\n")
  else()
    message(STATUS "As expected the file ${pl${PLG}File} did not change.")
  endif()
endmacro()

macro(acquire_timestamps When)
  file(TIMESTAMP "${plAFile}" plA${When} "${timeformat}")
  file(TIMESTAMP "${plBFile}" plB${When} "${timeformat}")
  file(TIMESTAMP "${plCFile}" plC${When} "${timeformat}")
  file(TIMESTAMP "${plDFile}" plD${When} "${timeformat}")
  file(TIMESTAMP "${plEFile}" plE${When} "${timeformat}")
endmacro()


# Initial build
try_compile(MOC_PLUGIN
  "${mocPlugBinDir}"
  "${mocPlugSrcDir}"
  MocPlugin
  CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
              "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_PLUGIN)
  message(FATAL_ERROR "Initial build of mocPlugin failed. Output: ${output}")
endif()

# Get names of the output binaries
find_library(plAFile "PlugA" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
find_library(plBFile "PlugB" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
find_library(plCFile "PlugC" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
find_library(plDFile "PlugD" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)
find_library(plEFile "PlugE" PATHS "${mocPlugBinDir}/Debug" "${mocPlugBinDir}" NO_DEFAULT_PATH)

# To avoid a race condition where the library has the same timestamp
# as a source file and therefore gets rebuild
# - sleep to ensure a timestamp change
# - rebuild library to ensure it has a new timestamp
sleep()
message(STATUS "Rebuilding library files to ensure new timestamps")
rebuild(1)


# - Ensure that the timestamp will change.
# - Change the json files referenced by Q_PLUGIN_METADATA
# - Rebuild
acquire_timestamps(Before)
sleep()
message(STATUS "Changing json files.")
configure_file("${mocPlugSrcDir}/jsonIn/StyleD.json" "${mocPlugBinDir}/jsonFiles/StyleC.json")
configure_file("${mocPlugSrcDir}/jsonIn/StyleE.json" "${mocPlugBinDir}/jsonFiles/sub/StyleD.json")
configure_file("${mocPlugSrcDir}/jsonIn/StyleC.json" "${mocPlugBinDir}/jsonFiles/StyleE.json")
sleep()
rebuild(2)
acquire_timestamps(After)
# Test changes
require_change_not(A)
require_change_not(B)
require_change(C)
require_change(D)
# There's a bug in Ninja on Windows:
# https://gitlab.kitware.com/cmake/cmake/issues/16776
if(NOT ("${CMAKE_GENERATOR}" MATCHES "Ninja"))
  require_change(E)
endif()


# - Ensure that the timestamp will change.
# - Change the json files referenced by A_CUSTOM_MACRO
# - Rebuild
acquire_timestamps(Before)
sleep()
message(STATUS "Changing json files")
configure_file("${mocPlugSrcDir}/jsonIn/StyleE.json" "${mocPlugBinDir}/jsonFiles/StyleC_Custom.json")
configure_file("${mocPlugSrcDir}/jsonIn/StyleC.json" "${mocPlugBinDir}/jsonFiles/sub/StyleD_Custom.json")
configure_file("${mocPlugSrcDir}/jsonIn/StyleD.json" "${mocPlugBinDir}/jsonFiles/StyleE_Custom.json")
sleep()
rebuild(3)
acquire_timestamps(After)
# Test changes
require_change_not(A)
require_change_not(B)
require_change(C)
require_change(D)
# There's a bug in Ninja on Windows
# https://gitlab.kitware.com/cmake/cmake/issues/16776
if(NOT ("${CMAKE_GENERATOR}" MATCHES "Ninja"))
  require_change(E)
endif()
