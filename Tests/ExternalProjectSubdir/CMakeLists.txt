cmake_minimum_required(VERSION 3.2)
project(ExternalProjectSubdir NONE)
include(ExternalProject)

ExternalProject_Add(Subdir1
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Subdir1
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Subdir1

  CMAKE_ARGS -DNORMAL_VAR=NORMAL_VALUE -DGENEX_VAR=$<1:GENEX_VALUE>
  LOG_CONFIGURE 1

  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  )
