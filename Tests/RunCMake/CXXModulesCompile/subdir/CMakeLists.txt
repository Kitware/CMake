cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_simple CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_subdirectory("with space")
