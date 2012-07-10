
cmake_minimum_required(VERSION 2.8)

project(lib_shared_and_static)

include(GenerateExportHeader)

add_compiler_export_flags()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lib_SRCS_CXX
  libshared_and_static.cpp
)

configure_file(libshared_and_static.cpp libshared_and_static.c COPYONLY)

set(lib_SRCS_C
  libshared_and_static.c
)

add_library(shared_variant SHARED ${lib_SRCS_CXX})
add_library(static_variant ${lib_SRCS_CXX})
add_library(shared_variant_c SHARED ${lib_SRCS_C})
set_property(TARGET shared_variant_c PROPERTY DEFINE_SYMBOL shared_variant_EXPORTS)
add_library(static_variant_c ${lib_SRCS_C})

generate_export_header(shared_variant BASE_NAME libshared_and_static)

set_target_properties(static_variant PROPERTIES COMPILE_FLAGS -DLIBSHARED_AND_STATIC_STATIC_DEFINE)
set_target_properties(static_variant_c PROPERTIES COMPILE_FLAGS -DLIBSHARED_AND_STATIC_STATIC_DEFINE)

export(TARGETS shared_variant static_variant shared_variant_c static_variant_c FILE Targets.cmake)
