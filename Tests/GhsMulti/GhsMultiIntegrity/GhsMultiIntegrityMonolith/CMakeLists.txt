# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_link_options("-non_shared")

project(test C)

# create virtual AS
add_executable(vas exe.c)
target_link_libraries(vas lib)
add_library(lib func.c)

# create kernel
add_executable(kernel kernel.c)
target_link_options(kernel PRIVATE -kernel)

# create monolith INTEGRITY application
add_executable(monolith test.int)
add_dependencies(monolith vas)
