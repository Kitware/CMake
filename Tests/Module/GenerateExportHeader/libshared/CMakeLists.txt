
cmake_minimum_required(VERSION 2.8)

project(libshared)

include(GenerateExportHeader)

add_compiler_export_flags(CXX CXX_EXPORT_FLAGS C C_EXPORT_FLAGS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(libshared SHARED libshared.cpp)
set_property(TARGET libshared PROPERTY COMPILE_FLAGS ${CXX_EXPORT_FLAGS})

configure_file(libshared.cpp libshared.c COPYONLY)

add_library(libshared_c SHARED libshared.c)
set_property(TARGET libshared_c PROPERTY DEFINE_SYMBOL libshared_EXPORTS)
set_property(TARGET libshared_c PROPERTY COMPILE_FLAGS ${C_EXPORT_FLAGS})

generate_export_header(libshared)

export(TARGETS libshared libshared_c FILE Targets.cmake)
