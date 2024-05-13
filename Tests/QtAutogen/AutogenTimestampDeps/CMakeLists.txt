cmake_minimum_required(VERSION 3.28)
project(AutogenTimestampDeps)
include("../AutogenCoreTest.cmake")

set(CMAKE_AUTOMOC ON)

add_custom_target(ProjectInfo
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UpdateProjectInfo.cmake
  BYPRODUCTS ${CMAKE_BINARY_DIR}/ProjectInfo.hpp)

set(ext_lib ${CMAKE_CURRENT_BINARY_DIR}/ext-build/${CMAKE_STATIC_LIBRARY_PREFIX}ext${CMAKE_STATIC_LIBRARY_SUFFIX})
include(ExternalProject)
ExternalProject_Add(ext_target
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/ext-build"
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS "${ext_lib}"
  )
add_library(ext STATIC IMPORTED)
set_property(TARGET ext PROPERTY IMPORTED_LOCATION "${ext_lib}")
add_dependencies(ext ext_target)

add_subdirectory(src)
