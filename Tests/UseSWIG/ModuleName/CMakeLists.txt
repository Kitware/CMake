cmake_minimum_required(VERSION 3.1...3.14)

project(TestModuleName CXX)

include(CTest)

find_package(SWIG REQUIRED)
cmake_policy(SET CMP0086 NEW)
include(${SWIG_USE_FILE})

find_package(Python REQUIRED COMPONENTS Interpreter Development)

unset(CMAKE_SWIG_FLAGS)

set_property(SOURCE "example.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "example.i" PROPERTY COMPILE_OPTIONS -includeall)
set_property(SOURCE "example.i" PROPERTY SWIG_MODULE_NAME new_example)

swig_add_library(example1
                 LANGUAGE python
                 OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/example1"
                 SOURCES example.i ../example.cxx)
set_target_properties (example1 PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/.."
  SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
  OUTPUT_NAME new_example
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/example1"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/example1"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/example1")
target_link_libraries(example1 PRIVATE Python::Module)


add_test (NAME ModuleName.example1
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_FILE_DIR:example1>"
  "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runme.py")
