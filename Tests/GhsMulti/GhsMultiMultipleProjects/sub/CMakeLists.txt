# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test2 C)

add_library(lib2 lib2.c)
add_executable(exe2 exe2.c)
target_link_libraries(exe2 lib1 lib2)
