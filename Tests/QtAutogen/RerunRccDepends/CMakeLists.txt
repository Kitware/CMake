cmake_minimum_required(VERSION 3.16)
project(RerunRccDepends)
include("../AutogenCoreTest.cmake")

# Tests rcc rebuilding when a resource file changes
# When a .qrc or a file listed in a .qrc file changes,
# the target must be rebuilt

# Dummy executable to generate a clean target
add_executable(dummy dummy.cpp)

# Utility variables
set(timeformat "%Y.%j.%H.%M%S")
set(rccDepSD "${CMAKE_CURRENT_SOURCE_DIR}/RccDepends")
set(rccDepBD "${CMAKE_CURRENT_BINARY_DIR}/RccDepends")

# Utility macros
macro(sleep)
  message(STATUS "Sleeping for a few seconds.")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
endmacro()

macro(acquire_timestamps When)
  file(TIMESTAMP "${rccDepBinPlain}" rdPlain${When} "${timeformat}")
  file(TIMESTAMP "${rccDepBinGenerated}" rdGenerated${When} "${timeformat}")
endmacro()

macro(rebuild buildName)
  message(STATUS "Starting build ${buildName} of rccDepends.")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${rccDepBD}"
    RESULT_VARIABLE result)
  if (result)
    message(FATAL_ERROR "Build ${buildName} of rccDepends failed.")
  else()
    message(STATUS "Build ${buildName} of rccDepends finished.")
  endif()
endmacro()

macro(require_change type)
  if (rd${type}After VERSION_GREATER rd${type}Before)
    message(STATUS "As expected the ${type} .qrc file ${rccDepBin${type}} changed.")
  else()
    message(SEND_ERROR "Unexpectedly the ${type} .qrc file ${rccDepBin${type}} did not change!\nTimestamp pre: ${rd${type}Before}\nTimestamp aft: ${rd${type}After}\n")
  endif()
endmacro()

macro(require_change_not type)
  if (rd${type}After VERSION_GREATER rd${type}Before)
    message(SEND_ERROR "Unexpectedly the ${type} .qrc file ${rccDepBin${type}} changed!\nTimestamp pre: ${rd${type}Before}\nTimestamp aft: ${rd${type}After}\n")
  else()
    message(STATUS "As expected the ${type} .qrc file ${rccDepBin${type}} did not change.")
  endif()
endmacro()


# Initial configuration
configure_file(${rccDepSD}/resPlainA.qrc.in ${rccDepBD}/resPlain.qrc COPYONLY)
configure_file(${rccDepSD}/resGenA.qrc.in ${rccDepBD}/resGen.qrc.in COPYONLY)

# Initial build
try_compile(RCC_DEPENDS
  "${rccDepBD}"
  "${rccDepSD}"
  RccDepends
  CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
              "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
              "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  OUTPUT_VARIABLE output
)
if (NOT RCC_DEPENDS)
  message(FATAL_ERROR "Initial build of rccDepends failed. Output: ${output}")
endif()

# Get name of the output binaries
file(STRINGS "${rccDepBD}/targetPlain.txt" targetListPlain ENCODING UTF-8)
file(STRINGS "${rccDepBD}/targetGen.txt" targetListGen ENCODING UTF-8)
list(GET targetListPlain 0 rccDepBinPlain)
list(GET targetListGen 0 rccDepBinGenerated)
message(STATUS "Target that uses a plain .qrc file is:\n  ${rccDepBinPlain}")
message(STATUS "Target that uses a GENERATED .qrc file is:\n  ${rccDepBinGenerated}")

# To avoid a race condition where the binary has the same timestamp
# as a source file and therefore gets rebuild
# - sleep to ensure a timestamp change
# - touch binary to ensure it has a new timestamp
acquire_timestamps(Before)
sleep()
message(STATUS "Touching binary files to ensure new timestamps")
file(TOUCH_NOCREATE "${rccDepBinPlain}" "${rccDepBinGenerated}")
acquire_timestamps(After)
require_change(Plain)
require_change(Generated)


# - Ensure that the timestamp will change
# - Change a resource files listed in the .qrc file
# - Rebuild
acquire_timestamps(Before)
sleep()
message(STATUS "Changing a resource file listed in the .qrc file")
file(TOUCH "${rccDepBD}/resPlain/input.txt" "${rccDepBD}/resGen/input.txt")
sleep()
rebuild(2)
acquire_timestamps(After)
# - Test if timestamps changed
require_change(Plain)
require_change(Generated)


# - Ensure that the timestamp will change
# - Change the .qrc file
# - Rebuild
acquire_timestamps(Before)
sleep()
message(STATUS "Changing the .qrc file")
configure_file(${rccDepSD}/resPlainB.qrc.in ${rccDepBD}/resPlain.qrc COPYONLY)
configure_file(${rccDepSD}/resGenB.qrc.in ${rccDepBD}/resGen.qrc.in COPYONLY)
sleep()
rebuild(3)
acquire_timestamps(After)
# - Test if timestamps changed
require_change(Plain)
require_change(Generated)


# - Ensure that the timestamp will change
# - Change a newly added resource files listed in the .qrc file
# - Rebuild
acquire_timestamps(Before)
sleep()
message(STATUS "Changing a newly added resource file listed in the .qrc file")
file(TOUCH "${rccDepBD}/resPlain/inputAdded.txt" "${rccDepBD}/resGen/inputAdded.txt")
sleep()
rebuild(4)
acquire_timestamps(After)
# - Test if timestamps changed
require_change(Plain)
require_change(Generated)


# - Ensure that the timestamp will change
# - Change nothing
# - Rebuild
acquire_timestamps(Before)
sleep()
message(STATUS "Changing nothing in the .qrc file")
rebuild(5)
acquire_timestamps(After)
# - Test if timestamps changed
require_change_not(Plain)
require_change_not(Generated)
