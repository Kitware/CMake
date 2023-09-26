cmake_minimum_required(VERSION 3.5)
project(TestsWorkingDirectoryProj)

add_executable(WorkingDirectory main.c)

enable_testing()

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

file(REAL_PATH "${CMAKE_BINARY_DIR}" _real_binary_dir)

add_test(NAME WorkingDirectory0 COMMAND WorkingDirectory "${_real_binary_dir}")

add_test(NAME WorkingDirectory1 COMMAND WorkingDirectory "${_real_binary_dir}")
set_tests_properties(WorkingDirectory1 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${_real_binary_dir}")

add_test(NAME WorkingDirectory2 COMMAND WorkingDirectory "${_parent_dir}")
set_tests_properties(WorkingDirectory2 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/.."
)

set(_default_cwd "${_real_binary_dir}")

# FIXME: How to deal with /debug, /release, etc. with VS or Xcode?
if(${CMAKE_GENERATOR} MATCHES "Makefiles")
add_test(WorkingDirectory3 ${EXECUTABLE_OUTPUT_PATH}/WorkingDirectory ${_default_cwd})
endif()

add_test(NAME WorkingDirectory4 WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND WorkingDirectory ${_real_binary_dir})

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${_real_binary_dir}")

add_test(NAME WorkingDirectory5 WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/.. COMMAND WorkingDirectory ${_parent_dir})

# FIXME: How to deal with /debug, /release, etc. with VS or Xcode?
if(${CMAKE_GENERATOR} MATCHES "Makefiles")
add_test(WorkingDirectory6 ${EXECUTABLE_OUTPUT_PATH}/WorkingDirectory ${_default_cwd} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)
endif()

add_subdirectory(subdir)
