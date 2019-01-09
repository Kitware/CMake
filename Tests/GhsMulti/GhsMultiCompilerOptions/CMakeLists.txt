# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

message("Copy project")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/CMakeLists.txt COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.c
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src
)

message("Building project")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(RESULT
  ${CMAKE_CURRENT_BINARY_DIR}/build
  ${CMAKE_CURRENT_BINARY_DIR}/src
  test
  CMAKE_FLAGS
    -DRUN_TEST=${RUN_TEST}
    -DCMAKE_BUILD_TYPE=${RUN_TEST_BUILD_TYPE}
  OUTPUT_VARIABLE OUTPUT)

message("Output from build:\n${OUTPUT}")
if (RUN_TEST STREQUAL "RELEASE_FLAGS")
  find_file (fileName test_none.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/build
    ${CMAKE_CURRENT_BINARY_DIR}/build/test_none
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(opt "-unexpected_release_option")
  string(FIND "${fileText}" "${opt}" opt_found)
  if ( NOT opt_found EQUAL -1 )
    message(SEND_ERROR "Release option found: ${opt}")
  endif()
else()
  unset(fileName CACHE)
  find_file (fileName K1.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/build
    ${CMAKE_CURRENT_BINARY_DIR}/build/K1
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(opt "-required-debug-option")
  string(FIND "${fileText}" "${opt}" opt_found)
  if ( opt_found EQUAL -1 )
    message(SEND_ERROR "Missing debug option: ${opt}")
  endif()

  unset(fileName CACHE)
  find_file (fileName K2.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/build
    ${CMAKE_CURRENT_BINARY_DIR}/build/K2
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(opt "-required-debug-option")
  string(FIND "${fileText}" "${opt}" opt_found)
  if ( opt_found EQUAL -1 )
    message(SEND_ERROR "Missing debug option: ${opt}")
  endif()

  unset(fileName CACHE)
  find_file (fileName K3.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/build
    ${CMAKE_CURRENT_BINARY_DIR}/build/K3
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(opt "-required-debug-option")
  string(FIND "${fileText}" "${opt}" opt_found)
  if ( opt_found EQUAL -1 )
    message(SEND_ERROR "Missing debug option: ${opt}")
  endif()

  unset(fileName CACHE)
  find_file (fileName K4.gpj
    ${CMAKE_CURRENT_BINARY_DIR}/build
    ${CMAKE_CURRENT_BINARY_DIR}/build/K4
    )
  message("Parsing project file: ${fileName}")
  file(STRINGS ${fileName} fileText)
  set(opt "-required-debug-option")
  string(FIND "${fileText}" "${opt}" opt_found)
  if ( opt_found EQUAL -1 )
    message(SEND_ERROR "Missing debug option: ${opt}")
  endif()
endif()
