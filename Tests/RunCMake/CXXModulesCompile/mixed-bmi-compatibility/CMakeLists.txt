cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_mixed_bmi_compatibility CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(importable)
target_sources(importable
  PUBLIC FILE_SET CXX_MODULES FILES importable.cxx)

target_compile_features(importable PUBLIC cxx_std_20)

add_executable(consumer20 consumer20.cxx)
target_compile_features(consumer20 PRIVATE cxx_std_20)
target_link_libraries(consumer20 PRIVATE importable)

add_executable(consumer23 consumer23.cxx)
target_compile_features(consumer23 PRIVATE cxx_std_23)
target_link_libraries(consumer23 PRIVATE importable)

add_test(NAME consumer20 COMMAND consumer20)
add_test(NAME consumer23 COMMAND consumer23)
