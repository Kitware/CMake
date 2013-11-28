cmake_minimum_required(VERSION 2.8)

project(WriteCompilerDetectionHeader)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(WriteCompilerDetectionHeader)

write_compiler_detection_header(
  FILE "${CMAKE_CURRENT_BINARY_DIR}/test_compiler_detection.h"
  PREFIX TEST
  COMPILERS GNU Clang MSVC
  FEATURES
    ${CMAKE_CXX_KNOWN_FEATURES}
)

macro(set_defines target true_defs false_defs)
  set(defines)
  foreach(def ${true_defs})
    list(APPEND defines ${def}=1)
  endforeach()
  foreach(def ${false_defs})
    list(APPEND defines ${def}=0)
  endforeach()
  target_compile_definitions(${target}
    PRIVATE
      ${defines}
  )
endmacro()

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU
    OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  list(APPEND true_defs EXPECTED_COMPILER_CXX_BINARY_LITERALS)
  list(APPEND false_defs EXPECTED_COMPILER_MSVCXX_SEALED)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  list(APPEND false_defs EXPECTED_COMPILER_CXX_BINARY_LITERALS)
  list(APPEND true_defs EXPECTED_COMPILER_MSVCXX_SEALED)
else()
  list(APPEND false_defs EXPECTED_COMPILER_CXX_BINARY_LITERALS)
  list(APPEND false_defs EXPECTED_COMPILER_MSVCXX_SEALED)
endif()

list(APPEND false_defs EXPECTED_COMPILER_GNUXX_TYPEOF)
list(APPEND false_defs EXPECTED_COMPILER_CXX_DELEGATING_CONSTRUCTORS)
list(APPEND false_defs EXPECTED_COMPILER_CXX_VARIADIC_TEMPLATES)

add_executable(WriteCompilerDetectionHeader main.cpp)
set_property(TARGET WriteCompilerDetectionHeader PROPERTY CXX_STANDARD 98)
set_defines(WriteCompilerDetectionHeader "${true_defs}" "${false_defs}")

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU
    OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  list(APPEND true_defs EXPECTED_COMPILER_GNUXX_TYPEOF)
  list(REMOVE_ITEM false_defs EXPECTED_COMPILER_GNUXX_TYPEOF)
endif()

add_executable(WriteCompilerDetectionHeader_98ext main.cpp)
set_property(TARGET WriteCompilerDetectionHeader_98ext PROPERTY CXX_STANDARD 98)
set_property(TARGET WriteCompilerDetectionHeader_98ext PROPERTY CXX_EXTENSIONS 1)
set_defines(WriteCompilerDetectionHeader_98ext "${true_defs}" "${false_defs}")

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU
    OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  list(APPEND false_defs  EXPECTED_COMPILER_GNUXX_TYPEOF)
  list(REMOVE_ITEM true_defs EXPECTED_COMPILER_GNUXX_TYPEOF)
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL GNU
      AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7.0)
    OR (CMAKE_CXX_COMPILER_ID STREQUAL Clang
      AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.0.0))
  list(APPEND true_defs EXPECTED_COMPILER_CXX_DELEGATING_CONSTRUCTORS)
  list(REMOVE_ITEM false_defs EXPECTED_COMPILER_CXX_DELEGATING_CONSTRUCTORS)
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL GNU
      AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7.0)
    OR (CMAKE_CXX_COMPILER_ID STREQUAL Clang
      AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.0.0))
  list(APPEND true_defs EXPECTED_COMPILER_CXX_VARIADIC_TEMPLATES)
  list(REMOVE_ITEM false_defs EXPECTED_COMPILER_CXX_VARIADIC_TEMPLATES)
endif()

add_executable(WriteCompilerDetectionHeader_11 main.cpp)
set_property(TARGET WriteCompilerDetectionHeader_11 PROPERTY CXX_STANDARD 11)
set_defines(WriteCompilerDetectionHeader_11 "${true_defs}" "${false_defs}")

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU
    OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  list(APPEND true_defs EXPECTED_COMPILER_GNUXX_TYPEOF)
  list(REMOVE_ITEM false_defs EXPECTED_COMPILER_GNUXX_TYPEOF)
endif()

add_executable(WriteCompilerDetectionHeader_11ext main.cpp)
set_property(TARGET WriteCompilerDetectionHeader_11ext PROPERTY CXX_STANDARD 11)
set_property(TARGET WriteCompilerDetectionHeader_11ext PROPERTY CXX_EXTENSIONS 1)
set_defines(WriteCompilerDetectionHeader_11ext "${true_defs}" "${false_defs}")
