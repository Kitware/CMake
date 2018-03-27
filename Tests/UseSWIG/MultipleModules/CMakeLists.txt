cmake_minimum_required(VERSION 3.1)

project(TestMultipleModules CXX)

include(CTest)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Python REQUIRED COMPONENTS Development)

find_package(PerlLibs REQUIRED)

unset(CMAKE_SWIG_FLAGS)

set_property(SOURCE "../example.i" PROPERTY CPLUSPLUS ON)
set_property(SOURCE "../example.i" PROPERTY COMPILE_OPTIONS -includeall)

set_property(SOURCE "../example.i"
  PROPERTY GENERATED_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/..")

swig_add_library(example1
                 LANGUAGE python
                 OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Python"
                 SOURCES ../example.i ../example.cxx)
target_link_libraries(example1 PRIVATE Python::Python)

# re-use sample interface file for another plugin
set_property(SOURCE "../example.i" APPEND PROPERTY
  GENERATED_INCLUDE_DIRECTORIES ${PERL_INCLUDE_PATH})
separate_arguments(c_flags UNIX_COMMAND "${PERL_EXTRA_C_FLAGS}")
set_property(SOURCE "../example.i" PROPERTY GENERATED_COMPILE_OPTIONS ${c_flags})

swig_add_library(example2
                 LANGUAGE perl
                 OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Perl"
                 SOURCES ../example.i ../example.cxx)
target_link_libraries(example2 PRIVATE ${PERL_LIBRARY})
