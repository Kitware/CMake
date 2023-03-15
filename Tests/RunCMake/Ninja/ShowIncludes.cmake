# Create a project to do showIncludes prefix detection.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt" "
cmake_minimum_required(VERSION 3.25)
project(ShowIncludes NONE)
include(CMakeDetermineCompilerId)
set(CMAKE_dummy_COMPILER \"${showIncludes}\")
CMAKE_DETERMINE_MSVC_SHOWINCLUDES_PREFIX(dummy \"\")
set(CMAKE_CL_SHOWINCLUDES_PREFIX \"\${CMAKE_dummy_CL_SHOWINCLUDES_PREFIX}\")
file(WRITE \"\${CMAKE_CURRENT_BINARY_DIR}/showIncludes.txt\" \"\${CMAKE_CL_SHOWINCLUDES_PREFIX}\")
")

if(RunCMake_MAKE_PROGRAM)
  set(maybe_MAKE_PROGRAM "-DRunCMake_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}")
endif()

# Run cmake in a new Window to isolate its console code page.
execute_process(COMMAND cmd /c start /min /wait ""
  ${CMAKE_COMMAND} -DCODEPAGE=${CODEPAGE} -DVSLANG=${VSLANG} ${maybe_MAKE_PROGRAM} -P ${CMAKE_CURRENT_LIST_DIR}/ShowIncludes-cmake.cmake)

# Print our internal UTF-8 representation of the showIncludes prefix.
file(READ "${CMAKE_CURRENT_BINARY_DIR}/showIncludes.txt" showIncludes_txt)
message(STATUS "showIncludes='${showIncludes_txt}'")
