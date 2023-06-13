cmake_minimum_required(VERSION 3.1...3.13)

project(TestModuleVersion2 CXX)

include(CTest)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

if (WIN32)
  set (PS $<SEMICOLON>)
else()
  set (PS ":")
endif()

set (UseSWIG_MODULE_VERSION 2)
unset(CMAKE_SWIG_FLAGS)

set_property(SOURCE "../example.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "../example.i" PROPERTY COMPILE_OPTIONS -includeall)

set_property(SOURCE "../example.i"
  PROPERTY GENERATED_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/..")

if(CMake_TEST_FindPython2)
  find_package(Python2 REQUIRED COMPONENTS Interpreter Development)

  swig_add_library(example1
                   LANGUAGE python
                   SOURCES ../example.i ../example.cxx)
  set_target_properties (example1 PROPERTIES
    OUTPUT_NAME example
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python2-1"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python2-1"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python2-1")
  target_link_libraries(example1 PRIVATE Python2::Module)

  add_test (NAME ModuleVersion2.example1
    COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_PROPERTY:example1,SWIG_SUPPORT_FILES_DIRECTORY>${PS}$<TARGET_FILE_DIR:example1>"
    "${Python2_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../runme.py")

  # re-use sample interface file for another plugin
  swig_add_library(example2
                   LANGUAGE python
                   SOURCES ../example.i ../example.cxx)
  set_target_properties (example2 PROPERTIES
    OUTPUT_NAME example
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python2-2"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python2-2"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python2-2")
  target_link_libraries(example2 PRIVATE Python2::Module)

  add_test (NAME ModuleVersion2.example2
    COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_PROPERTY:example2,SWIG_SUPPORT_FILES_DIRECTORY>${PS}$<TARGET_FILE_DIR:example2>"
    "${Python2_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../runme.py")
endif()

if(CMake_TEST_FindPython3)
  find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

  swig_add_library(example3
                   LANGUAGE python
                   SOURCES ../example.i ../example.cxx)
  set_target_properties (example3 PROPERTIES
    OUTPUT_NAME example
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python3-1"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python3-1"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python3-1")
  target_link_libraries(example3 PRIVATE Python3::Module)

  add_test (NAME ModuleVersion2.example3
    COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_PROPERTY:example3,SWIG_SUPPORT_FILES_DIRECTORY>${PS}$<TARGET_FILE_DIR:example3>"
    "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../runme.py")


  # re-use sample interface file for another plugin
  swig_add_library(example4
                   LANGUAGE python
                   SOURCES ../example.i ../example.cxx)
  set_target_properties (example2 PROPERTIES
    OUTPUT_NAME example
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python3-2"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python3-2"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Python3-2")
  target_link_libraries(example4 PRIVATE Python3::Module)

  add_test (NAME ModuleVersion2.example4
    COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=$<TARGET_PROPERTY:example4,SWIG_SUPPORT_FILES_DIRECTORY>${PS}$<TARGET_FILE_DIR:example4>"
    "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../runme.py")
endif()
