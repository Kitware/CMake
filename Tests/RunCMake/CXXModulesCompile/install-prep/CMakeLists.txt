cmake_minimum_required(VERSION 4.4)
project(cxx_modules_install_prep_sources CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated.cxx"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx"
  COMMAND "${CMAKE_COMMAND}"
          -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx"
          "${CMAKE_CURRENT_BINARY_DIR}/generated.cxx")

add_library(generated_module STATIC EXCLUDE_FROM_ALL)
target_sources(generated_module
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/generated.cxx")
target_compile_features(generated_module PUBLIC cxx_std_20)

install(TARGETS generated_module
  ARCHIVE DESTINATION "artifacts" COMPONENT Artifacts EXCLUDE_FROM_ALL
  FILE_SET CXX_MODULES DESTINATION "sources" COMPONENT Sources)
