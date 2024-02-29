cmake_minimum_required(VERSION 3.28)
project(AutogenTimestampDeps)
include("../AutogenCoreTest.cmake")

set(CMAKE_AUTOMOC ON)

add_custom_target(ProjectInfo
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UpdateProjectInfo.cmake
  BYPRODUCTS ${CMAKE_BINARY_DIR}/ProjectInfo.hpp)

add_subdirectory(src)
