cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_internal_partitions CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

include(GenerateExportHeader)

add_library(internal-partitions)
generate_export_header(internal-partitions)
target_sources(internal-partitions
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/internal-partitions_export.h"
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx
        partition.cxx
        internal.cxx)
target_compile_features(internal-partitions PUBLIC cxx_std_20)

add_executable(exe)
target_link_libraries(exe PRIVATE internal-partitions)
target_sources(exe
  PRIVATE
    main.cxx)

add_test(NAME exe COMMAND exe)
