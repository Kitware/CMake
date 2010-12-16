cmake_minimum_required(VERSION 2.6)
project(TestsWorkingDirectoryProj)

add_executable(WorkingDirectory main.cxx)

enable_testing()

add_test(NAME WorkingDirectory1 COMMAND WorkingDirectory)
add_test(NAME WorkingDirectory2 COMMAND WorkingDirectory)
add_test(WorkingDirectory3 WorkingDirectory)

set_tests_properties(WorkingDirectory1 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  PASS_REGULAR_EXPRESSION "Working directory: ${CMAKE_BINARY_DIR}"
)

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${CMAKE_BINARY_DIR}")

set_tests_properties(WorkingDirectory2 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/.."
  PASS_REGULAR_EXPRESSION "Working directory: ${_parent_dir}"
)

string(REGEX REPLACE "/[^/]*$" "" _wd_exe "${CMAKE_BINARY_DIR}")
get_filename_component(_default_cwd "${_wd_exe}" ABSOLUTE)

set_tests_properties(WorkingDirectory3 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: ${_default_cwd}"
)

add_test(NAME WorkingDirectory4 WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND WorkingDirectory)
add_test(NAME WorkingDirectory5 WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/.. COMMAND WorkingDirectory)
add_test(WorkingDirectory6 WorkingDirectory WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)

set_tests_properties(WorkingDirectory4 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: ${CMAKE_BINARY_DIR}"
)

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${CMAKE_BINARY_DIR}")

set_tests_properties(WorkingDirectory5 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: ${_parent_dir}"
)

string(REGEX REPLACE "/[^/]*$" "" _wd_exe "${CMAKE_BINARY_DIR}")
get_filename_component(_default_cwd "${_wd_exe}" ABSOLUTE)

set_tests_properties(WorkingDirectory6 PROPERTIES
  PASS_REGULAR_EXPRESSION "Working directory: ${_default_cwd}"
)
