cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_library CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

include(GenerateExportHeader)

add_library(library)
generate_export_header(library)
target_sources(library
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/library_export.h"
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        importable.cxx)
target_compile_features(library PUBLIC cxx_std_20)

add_executable(exe)
target_link_libraries(exe PRIVATE library)
target_sources(exe
  PRIVATE
    main.cxx)

add_test(NAME exe COMMAND exe)
