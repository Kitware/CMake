cmake_minimum_required(VERSION 3.18)

project(TestMultipleFiles CXX)

find_package(SWIG REQUIRED)
include(UseSWIG)

unset(SWIG_LANG_TYPE)
unset(SWIG_LANG_INCLUDE_DIRECTORIES)
unset(SWIG_LANG_DEFINITIONS)
unset(SWIG_LANG_OPTIONS)
unset(SWIG_LANG_LIBRARIES)

find_package(Python REQUIRED COMPONENTS Development)

set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/add.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/sub.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/add.i" PROPERTY SWIG_MODULE_NAME _add)
set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/sub.i" PROPERTY SWIG_MODULE_NAME _sub)


swig_add_library(example
                 LANGUAGE python
                 TYPE MODULE
                 SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/add.i"
                         "${CMAKE_CURRENT_SOURCE_DIR}/sub.i"
                         "${CMAKE_CURRENT_SOURCE_DIR}/add.cxx"
                         "${CMAKE_CURRENT_SOURCE_DIR}/sub.cxx")
target_include_directories(example PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(example PRIVATE Python::Module)
