# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test3 C)

add_library(lib3 lib3.c)
add_executable(exe3 exe3.c)
target_link_libraries(exe3 lib1 lib3)
