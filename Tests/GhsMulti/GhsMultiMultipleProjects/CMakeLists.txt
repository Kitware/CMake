# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

if(CMAKE_C_COMPILER_ID STREQUAL "GHS")
  add_link_options("-non_shared")
endif()

add_library(lib1 lib1.c)
add_executable(exe1 exe1.c)
target_link_libraries(exe1 lib1)

add_subdirectory(sub)
add_subdirectory(sub2 examples EXCLUDE_FROM_ALL)
