cmake_minimum_required(VERSION 3.28)
project(export_with_headers CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_with_headers)
target_sources(export_with_headers
  PUBLIC
    FILE_SET headers TYPE HEADERS
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/include/subdir/header.h"
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
    FILES
      importable.cxx)
target_compile_features(export_with_headers PUBLIC cxx_std_20)

install(TARGETS export_with_headers
  EXPORT CXXModules
  FILE_SET headers
  FILE_SET modules DESTINATION ${CMAKE_INSTALL_LIBDIR}/miu)
export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_with_headers-targets.cmake"
  CXX_MODULES_DIRECTORY "export_with_headers-cxx-modules")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_with_headers-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_with_headers-targets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)
")

set(generator
  -G "${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR_TOOLSET)
  list(APPEND generator
    -T "${CMAKE_GENERATOR_TOOLSET}")
endif ()
if (CMAKE_GENERATOR_PLATFORM)
  list(APPEND generator
    -A "${CMAKE_GENERATOR_PLATFORM}")
endif ()

add_test(NAME export_with_headers_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexport_with_headers_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test")
