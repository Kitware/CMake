cmake_minimum_required (VERSION 2.6)
PROJECT(example1)
ADD_EXECUTABLE(example1 example1.cxx)

ADD_CUSTOM_COMMAND(TARGET example1 POST_BUILD
  COMMAND "${CMAKE_COMMAND}" ARGS -E remove ${SUBDIR_BINARY_DIR}/ShouldBeHere
  COMMENT "Remove marker file that should exist because this should not be run")
