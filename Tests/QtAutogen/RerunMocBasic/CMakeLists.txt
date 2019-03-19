cmake_minimum_required(VERSION 3.10)
project(RerunMocBasic)
include("../AutogenCoreTest.cmake")

# Dummy executable to generate a clean target
add_executable(dummy dummy.cpp)

# Utility variables
set(timeformat "%Y.%j.%H.%M%S")
set(mocBasicSrcDir "${CMAKE_CURRENT_SOURCE_DIR}/MocBasic")
set(mocBasicBinDir "${CMAKE_CURRENT_BINARY_DIR}/MocBasic")

# Utility macros
macro(sleep)
  message(STATUS "Sleeping for a few seconds.")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
endmacro()

macro(acquire_timestamp When)
  file(TIMESTAMP "${mocBasicBin}" time${When} "${timeformat}")
endmacro()

macro(rebuild buildName)
  message(STATUS "Starting build ${buildName}.")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${mocBasicBinDir}" RESULT_VARIABLE result)
  if (result)
    message(FATAL_ERROR "Build ${buildName} failed.")
  else()
    message(STATUS "Build ${buildName} finished.")
  endif()
endmacro()

macro(require_change)
  if (timeAfter VERSION_GREATER timeBefore)
    message(STATUS "As expected the file ${mocBasicBin} changed.")
  else()
    message(SEND_ERROR "Unexpectedly the file ${mocBasicBin} did not change!\nTimestamp pre: ${timeBefore}\nTimestamp aft: ${timeAfter}\n")
  endif()
endmacro()

macro(require_change_not)
  if (timeAfter VERSION_GREATER timeBefore)
    message(SEND_ERROR "Unexpectedly the file ${mocBasicBin} changed!\nTimestamp pre: ${timeBefore}\nTimestamp aft: ${timeAfter}\n")
  else()
    message(STATUS "As expected the file ${mocBasicBin} did not change.")
  endif()
endmacro()


# Initial build
configure_file("${mocBasicSrcDir}/test1a.h.in" "${mocBasicBinDir}/test1.h" COPYONLY)
try_compile(MOC_RERUN
  "${mocBasicBinDir}"
  "${mocBasicSrcDir}"
  MocBasic
  CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
              "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_RERUN)
  message(FATAL_ERROR "Initial build of mocBasic failed. Output: ${output}")
endif()

# Get name of the output binary
file(STRINGS "${mocBasicBinDir}/mocBasic.txt" mocBasicList ENCODING UTF-8)
list(GET mocBasicList 0 mocBasicBin)

# To avoid a race condition where the binary has the same timestamp
# as a source file and therefore gets rebuild
# - sleep to ensure a timestamp change
# - touch binary to ensure it has a new timestamp
acquire_timestamp(Before)
sleep()
message(STATUS "Touching binary file to ensure a new timestamps")
file(TOUCH_NOCREATE "${mocBasicBin}")
acquire_timestamp(After)
require_change()


# - Ensure that the timestamp will change
# - Change header file content
# - Rebuild
acquire_timestamp(Before)
sleep()
message(STATUS "Changing the header content for a MOC re-run")
configure_file("${mocBasicSrcDir}/test1b.h.in" "${mocBasicBinDir}/test1.h" COPYONLY)
sleep()
rebuild(2)
acquire_timestamp(After)
require_change()


# - Ensure that the timestamp would change
# - Change nothing
# - Rebuild
acquire_timestamp(Before)
sleep()
message(STATUS "Changing nothing for no MOC re-run")
rebuild(3)
acquire_timestamp(After)
require_change_not()
