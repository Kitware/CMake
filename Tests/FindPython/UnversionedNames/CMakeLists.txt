cmake_minimum_required(VERSION 3.19...3.20)

project(UnversionedNames LANGUAGES NONE)

# check if it is possible to find python with a generic name
find_program(UNVERSIONED_Python3 NAMES python3)

if (NOT UNVERSIONED_Python3)
  # no generic name available
  # test cannot be done
  return()
endif()

# search with default configuration
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if (Python3_EXECUTABLE STREQUAL UNVERSIONED_Python3)
  # default configuration pick-up the generic name
  # test cannot be completed
  return()
endif()

unset(Python3_EXECUTABLE)
# Force now to search first for  generic name
set(Python3_FIND_UNVERSIONED_NAMES FIRST)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

if (NOT Python3_EXECUTABLE STREQUAL UNVERSIONED_Python3)
  message(SEND_ERROR "Found unexpected interpreter ${Python3_EXECUTABLE} instead of ${UNVERSIONED_Python3}")
endif()

# To check value 'NEVER", creates  directory holding a symlink to the generic name
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(CREATE_LINK "${UNVERSIONED_Python3}" "${CMAKE_CURRENT_BINARY_DIR}/bin/python3" SYMBOLIC)

unset(Python3_EXECUTABLE)
set(Python3_FIND_UNVERSIONED_NAMES FIRST)
set(Python3_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
# First search: generic name must be found
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if (NOT Python3_EXECUTABLE STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/bin/python3")
  message(FATAL_ERROR "Found unexpected interpreter ${Python3_EXECUTABLE} instead of ${CMAKE_CURRENT_BINARY_DIR}/bin/python3")
endif()

unset(Python3_EXECUTABLE)
set(Python3_FIND_UNVERSIONED_NAMES LAST)

# Second search: generic name must be found
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if (NOT Python3_EXECUTABLE STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/bin/python3")
  message(FATAL_ERROR "Found unexpected interpreter ${Python3_EXECUTABLE} instead of ${CMAKE_CURRENT_BINARY_DIR}/bin/python3")
endif()

unset(Python3_EXECUTABLE)
set(Python3_FIND_UNVERSIONED_NAMES NEVER)

# Third search: generic name must NOT be found
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if (Python3_EXECUTABLE STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/bin/python3")
  message(FATAL_ERROR "Found unexpected interpreter ${Python3_EXECUTABLE}")
endif()
