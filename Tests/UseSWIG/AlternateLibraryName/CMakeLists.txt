cmake_minimum_required(VERSION 3.14...3.15)

project(TestAlternateLibraryName CXX)

include(CTest)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Python2 REQUIRED COMPONENTS Interpreter Development)

# Path separator
if (WIN32)
  set (PS "$<SEMICOLON>")
else()
  set (PS ":")
endif()

unset(CMAKE_SWIG_FLAGS)

set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../example.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../example.i" PROPERTY COMPILE_OPTIONS -includeall)

swig_add_library(example_python
                 LANGUAGE python
                 SOURCES ../example.i ../example.cxx)
set_target_properties (example_python PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/.."
  SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
target_link_libraries(example_python PRIVATE Python2::Python)


add_test (NAME AlternateLibraryName.example1
  COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}${PS}$<TARGET_FILE_DIR:example_python>"
  "${Python2_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../runme.py")
