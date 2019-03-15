cmake_minimum_required(VERSION 3.6)
project(ExternalProjectSourceSubdirNotCMake NONE)
include(ExternalProject)

find_program(MAKE_EXECUTABLE
  NAMES gmake make)

if (NOT MAKE_EXECUTABLE)
  message("No `make` executable found; skipping")
  return ()
endif ()

ExternalProject_Add(Example
  SOURCE_SUBDIR subdir
  BUILD_IN_SOURCE 1
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Example
  CONFIGURE_COMMAND ""
  BUILD_COMMAND "${MAKE_EXECUTABLE}"
  INSTALL_COMMAND ""
  )
