cmake_minimum_required(VERSION 3.9)
project(${RunCMake_TEST} NONE)

# Tests assume no previous downloads in the output directory
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/_deps)

if(CMP0168 STREQUAL "NEW")
  cmake_policy(SET CMP0168 NEW)
  string(REGEX REPLACE "-direct$" "" RunCMake_TEST "${RunCMake_TEST}")
else()
  cmake_policy(SET CMP0168 OLD)
endif()
include(${RunCMake_TEST}.cmake)
