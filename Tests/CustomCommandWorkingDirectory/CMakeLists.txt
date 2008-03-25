cmake_minimum_required (VERSION 2.6)
PROJECT(TestWorkingDir)

ADD_CUSTOM_COMMAND(
  OUTPUT "${TestWorkingDir_BINARY_DIR}/working.c"
  COMMAND "${CMAKE_COMMAND}" -E copy ./working.c.in "${TestWorkingDir_BINARY_DIR}/working.c"
  WORKING_DIRECTORY "${TestWorkingDir_SOURCE_DIR}"
  COMMENT "custom command"
)

SET_SOURCE_FILES_PROPERTIES(
  "${TestWorkingDir_BINARY_DIR}/working.c" 
  "${TestWorkingDir_BINARY_DIR}/customTarget.c" 
  PROPERTIES GENERATED 1)

ADD_EXECUTABLE(working "${TestWorkingDir_BINARY_DIR}/working.c" 
  "${TestWorkingDir_BINARY_DIR}/customTarget.c")

ADD_CUSTOM_TARGET(
  Custom ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different ./customTarget.c "${TestWorkingDir_BINARY_DIR}/customTarget.c"
  WORKING_DIRECTORY  "${TestWorkingDir_SOURCE_DIR}"
)

ADD_DEPENDENCIES(working Custom)
