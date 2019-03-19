# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(RESULT
  ${CMAKE_CURRENT_BINARY_DIR}/build
  ${CMAKE_CURRENT_SOURCE_DIR}/test.c
  OUTPUT_VARIABLE OUTPUT
  COPY_FILE "${CMAKE_CURRENT_BINARY_DIR}/test_library"
)

message(STATUS "Output from build:\n${OUTPUT}")

if(NOT RESULT)
  message(SEND_ERROR "try_compile() failed")
endif()

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/test_library")
  if (IS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_library")
    message(SEND_ERROR "library is folder !")
  else()
    message(STATUS "library seems okay")
  endif()
else()
  message(SEND_ERROR "library is not found !")
endif()
