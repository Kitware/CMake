# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_executable(exe1 exe1.c)
target_link_libraries(exe1 lib1)
target_include_directories(exe1 PRIVATE "${test_BINARY_DIR}")
if(CMAKE_C_COMPILER_ID STREQUAL "GHS")
  target_link_options(exe1 PRIVATE "-non_shared")
endif()
