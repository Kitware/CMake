cmake_minimum_required(VERSION 2.6)
project(TestsWorkingDirectoryProj)

add_executable(WorkingDirectory main.cxx)

enable_testing()

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

add_test(NAME WorkingDirectory1 COMMAND WorkingDirectory)
add_test(NAME WorkingDirectory2 COMMAND WorkingDirectory)
add_test(WorkingDirectory3 WorkingDirectory)

set_tests_properties(WorkingDirectory1 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  PASS_REGULAR_EXPRESSION "Working directory: -->${CMAKE_BINARY_DIR}<--"
)

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${CMAKE_BINARY_DIR}")

set_tests_properties(WorkingDirectory2 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/.."
  PASS_REGULAR_EXPRESSION "Working directory: -->${_parent_dir}<--"
)

get_filename_component(_default_cwd "${EXECUTABLE_OUTPUT_PATH}" PATH)

# FIXME: How to deal with /debug, /release, etc. with VS or XCode?
if(${CMAKE_GENERATOR} MATCHES "Makefiles")
set_tests_properties(WorkingDirectory3 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: -->${_default_cwd}<--"
)
endif()

add_test(NAME WorkingDirectory4 WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND WorkingDirectory)
add_test(NAME WorkingDirectory5 WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/.. COMMAND WorkingDirectory)
add_test(WorkingDirectory6 WorkingDirectory WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)

set_tests_properties(WorkingDirectory4 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: -->${CMAKE_BINARY_DIR}<--"
)

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${CMAKE_BINARY_DIR}")

set_tests_properties(WorkingDirectory5 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: -->${_parent_dir}<--"
)

# FIXME: How to deal with /debug, /release, etc. with VS or XCode?
if(${CMAKE_GENERATOR} MATCHES "Makefiles")
set_tests_properties(WorkingDirectory6 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: -->${_default_cwd}<--"
)
endif()
