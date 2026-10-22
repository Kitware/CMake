cmake_minimum_required(VERSION 3.28)
project(no_cxx_scanning LANGUAGES C CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")
set(CMAKE_CXX_STANDARD 20)

add_library(c_only STATIC a.c)
add_library(config_only STATIC a.c
  "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/extra.cxx>")

# Even a C++ target need not have a collation step when its sources opt out.
set_source_files_properties(main.cxx PROPERTIES CXX_SCAN_FOR_MODULES OFF)
add_executable(main main.cxx)
target_link_libraries(main PRIVATE c_only config_only)

add_test(NAME main COMMAND main)
