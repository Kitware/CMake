# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

# Tests assume no previous builds in the build directory
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/build)

macro (test_output)
  if (BUILD_OUTPUT STREQUAL EXPECTED_LINES )
    message("Build OK")
  else()
    message("BUILD_OUTPUT")
    foreach(Line IN LISTS BUILD_OUTPUT)
      message("${Line}")
    endforeach()
    message("EXPECTED_LINES")
    foreach(Line IN LISTS EXPECTED_LINES)
      message("${Line}")
    endforeach()
    message(SEND_ERROR "Build KO")
  endif()
endmacro()

message("Copy project")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/CMakeLists.txt COPYONLY)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/exe1.c
          ${CMAKE_CURRENT_SOURCE_DIR}/lib1.c
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src
)

message("Building ALL target")
try_compile(RESULT
  ${CMAKE_CURRENT_BINARY_DIR}/build
  ${CMAKE_CURRENT_BINARY_DIR}/src
  test
  CMAKE_FLAGS
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
  OUTPUT_VARIABLE BUILD_OUTPUT)

message("Output from build:\n${BUILD_OUTPUT}")

#filter outputs
string(REPLACE "\r" "" BUILD_OUTPUT "${BUILD_OUTPUT}")
string(REPLACE "\n" ";" BUILD_OUTPUT "${BUILD_OUTPUT}")
list(FILTER BUILD_OUTPUT INCLUDE REGEX "^.*CT:")

unset(EXPECTED_LINES)
list(APPEND EXPECTED_LINES "CT: Processing target_empty_prebuild")
list(APPEND EXPECTED_LINES "CT: Processing target_empty_postbuild")
list(APPEND EXPECTED_LINES "CT: Processing target_cmd")
list(APPEND EXPECTED_LINES "CT: Processing target_cmd_prebuild")
list(APPEND EXPECTED_LINES "CT: Processing target_cmd_postbuild")

test_output()

message("Building target_update_files target")
try_compile(RESULT
  ${CMAKE_CURRENT_BINARY_DIR}/build
  ${CMAKE_CURRENT_BINARY_DIR}/src
  test target_update_files
  CMAKE_FLAGS
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
  OUTPUT_VARIABLE BUILD_OUTPUT)

message("Output from build:\n${BUILD_OUTPUT}")

#filter outputs
string(REPLACE "\r" "" BUILD_OUTPUT "${BUILD_OUTPUT}")
string(REPLACE "\n" ";" BUILD_OUTPUT "${BUILD_OUTPUT}")
list(FILTER BUILD_OUTPUT INCLUDE REGEX "^.*CT:")

unset(EXPECTED_LINES)
list(APPEND EXPECTED_LINES "CT: Processing target_empty_prebuild")
list(APPEND EXPECTED_LINES "CT: Processing target_empty_postbuild")
list(APPEND EXPECTED_LINES "CT: generate C file another_file")
list(APPEND EXPECTED_LINES "CT: generate text file dependsA")
list(APPEND EXPECTED_LINES "CT: generate text file out_of_order_dep")
list(APPEND EXPECTED_LINES "CT: generate text files A, B, and C")
list(APPEND EXPECTED_LINES "CT: Processing target_update_files")

test_output()

message("Rerun target_update_files target")
try_compile(RESULT
  ${CMAKE_CURRENT_BINARY_DIR}/build
  ${CMAKE_CURRENT_BINARY_DIR}/src
  test target_update_files
  CMAKE_FLAGS
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
  OUTPUT_VARIABLE BUILD_OUTPUT)

message("Output from build:\n${BUILD_OUTPUT}")

#filter outputs
string(REPLACE "\r" "" BUILD_OUTPUT "${BUILD_OUTPUT}")
string(REPLACE "\n" ";" BUILD_OUTPUT "${BUILD_OUTPUT}")
list(FILTER BUILD_OUTPUT INCLUDE REGEX "^.*CT:")

unset(EXPECTED_LINES)
list(APPEND EXPECTED_LINES "CT: Processing target_empty_prebuild")
list(APPEND EXPECTED_LINES "CT: Processing target_empty_postbuild")
list(APPEND EXPECTED_LINES "CT: generate text files A, B, and C")
list(APPEND EXPECTED_LINES "CT: Processing target_update_files")

test_output()
