cmake_minimum_required(VERSION 2.8)

project(src)

include(CheckCXXCompilerFlag)
include(GenerateExportHeader)

check_cxx_compiler_flag(-Wl,--no-undefined HAS_NO_UNDEFINED)

if (HAS_NO_UNDEFINED AND NOT APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
