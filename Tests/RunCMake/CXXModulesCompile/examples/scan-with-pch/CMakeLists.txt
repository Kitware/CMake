cmake_minimum_required(VERSION 3.28)
project(cxx_modules_scan_with_pch CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(simple)
target_sources(simple
  PRIVATE
    main.cxx)
target_compile_features(simple PUBLIC cxx_std_20)
target_precompile_headers(simple PRIVATE pch.h)

add_test(NAME simple COMMAND simple)
