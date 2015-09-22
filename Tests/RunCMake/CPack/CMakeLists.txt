cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${RunCMake_TEST} CXX)
include(${RunCMake_TEST}.cmake)

# include test generator specifics
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${GENERATOR_TYPE}/${RunCMake_TEST}-specifics.cmake")
  include("${GENERATOR_TYPE}/${RunCMake_TEST}-specifics.cmake")
endif()

set(CPACK_GENERATOR "${GENERATOR_TYPE}")
include(CPack)
