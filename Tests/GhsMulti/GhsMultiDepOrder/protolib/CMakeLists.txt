# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_custom_target(proto ALL
  DEPENDS proto1.c
          proto1.h
  SOURCES
          ${test_SOURCE_DIR}/protolib/proto1.c.in
          ${test_SOURCE_DIR}/protolib/proto1.h.in
  COMMENT "Creating proto files")

add_custom_command(
  OUTPUT proto1.c
  COMMAND ${CMAKE_COMMAND} -E copy
    ${test_SOURCE_DIR}/protolib/proto1.c.in proto1.c
  DEPENDS ${test_SOURCE_DIR}/protolib/proto1.c.in
  COMMENT "generate proto C files"
)

add_custom_command(
  OUTPUT proto1.h
  COMMAND ${CMAKE_COMMAND} -E copy
    ${test_SOURCE_DIR}/protolib/proto1.h.in proto1.h
  DEPENDS ${test_SOURCE_DIR}/protolib/proto1.h.in
  COMMENT "generate proto H files"
)
