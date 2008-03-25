cmake_minimum_required (VERSION 2.6)
PROJECT(Plugin)

# Test per-target output directory properties.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/plugin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/static)

# We need ansi C support.
IF(CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
ENDIF(CMAKE_ANSI_CFLAGS)

# We need the dynamic loader support from KWSys to load the plugin in
# the executable.
SET(KWSYS_NAMESPACE kwsys)
SET(KWSYS_HEADER_ROOT ${Plugin_BINARY_DIR}/include)
SET(KWSYS_USE_DynamicLoader 1)
ADD_SUBDIRECTORY(${Plugin_SOURCE_DIR}/../../Source/kwsys src/kwsys)

# Configure the location of plugins.
CONFIGURE_FILE(${Plugin_SOURCE_DIR}/src/example_exe.h.in
               ${Plugin_BINARY_DIR}/include/example_exe.h @ONLY)

# We need to include headers from the source tree and configured
# headers in the build tree.
INCLUDE_DIRECTORIES(
  ${Plugin_BINARY_DIR}/include
  ${Plugin_SOURCE_DIR}/include
  )

# Create an executable that exports an API for use by plugins.
ADD_EXECUTABLE(example_exe src/example_exe.cxx)
SET_TARGET_PROPERTIES(example_exe PROPERTIES
  ENABLE_EXPORTS 1
  OUTPUT_NAME example
  )
TARGET_LINK_LIBRARIES(example_exe kwsys)

# Create a plugin that uses the API provided by the executable.
# This module "links" to the executable to use the symbols.
ADD_LIBRARY(example_mod_1 MODULE src/example_mod_1.c)
TARGET_LINK_LIBRARIES(example_mod_1 example_exe)

# TODO:
#  - create a plugin that links to a static lib
#  - create a plugin that links to a shared lib
