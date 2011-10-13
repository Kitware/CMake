cmake_minimum_required(VERSION 2.8)

project(libs)

include(CheckCXXCompilerFlag)
include(GenerateExportHeader)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_library(libA SHARED classA.cpp)
add_library(libB SHARED classB.cpp)

generate_export_header(libA)
generate_export_header(libB)

target_link_libraries(libB libA)

target_link_libraries(libA LINK_INTERFACE_LIBRARIES "")
target_link_libraries(libB LINK_INTERFACE_LIBRARIES "")
