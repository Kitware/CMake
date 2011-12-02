
cmake_minimum_required(VERSION 2.8)

project(lib_shared_and_static)

include(GenerateExportHeader)

add_compiler_export_flags()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lib_SRCS
  libshared_and_static.cpp
)

add_library(shared_variant SHARED ${lib_SRCS})
add_library(static_variant ${lib_SRCS})

generate_export_header(shared_variant BASE_NAME libshared_and_static)

set_target_properties(static_variant PROPERTIES COMPILE_FLAGS -DLIBSHARED_AND_STATIC_STATIC_DEFINE)

export(TARGETS shared_variant static_variant FILE Targets.cmake)
