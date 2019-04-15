# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(test C)

#set_property( GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
add_subdirectory(exec)
add_subdirectory(lib)
add_subdirectory(protolib)
add_dependencies(lib1 proto)
