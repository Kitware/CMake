# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

add_library(lib1 STATIC
  func1.c lib1.h
  "${test_BINARY_DIR}/protolib/proto1.c"
  "${test_BINARY_DIR}/protolib/proto1.h")
set_source_files_properties(
  "${test_BINARY_DIR}/protolib/proto1.c"
  "${test_BINARY_DIR}/protolib/proto1.h"
  PROPERTIES GENERATED 1)
target_include_directories(lib1 PRIVATE "${test_BINARY_DIR}/protolib"
  PUBLIC .)
add_custom_command( TARGET lib1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${test_BINARY_DIR}/protolib/proto1.h" "${test_BINARY_DIR}/p.h"
  COMMENT "Copy ${test_BINARY_DIR}/protolib/proto1.h ${test_BINARY_DIR}/p.h"
  BYPRODUCTS "${test_BINARY_DIR}/p.h")
