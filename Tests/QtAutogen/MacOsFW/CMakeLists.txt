cmake_minimum_required(VERSION 3.16)
project(MacOsFW)
include("../AutogenGuiTest.cmake")

find_package(Qt${QT_TEST_VERSION}Test REQUIRED)

if(QT_TEST_VERSION EQUAL 5)
  set(CMAKE_CXX_STANDARD 11)
elseif(QT_TEST_VERSION EQUAL 6)
  set(CMAKE_CXX_STANDARD 17)
else()
  message(FATAL_ERROR "Unsupported Qt version: ${QT_TEST_VERSION}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_INSTALL_NAME_DIR ${CMAKE_BINARY_DIR}/output/lib)

set (CMAKE_MACOSX_RPATH OFF) # otherwise ON by default
cmake_policy(SET CMP0068 NEW)

add_subdirectory(src)
add_subdirectory(test)
