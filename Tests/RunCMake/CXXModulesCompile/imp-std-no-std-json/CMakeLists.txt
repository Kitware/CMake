cmake_minimum_required(VERSION 3.29)
project(cxx_modules_import_std_no_json CXX)

set(CMAKE_CXX_STDLIB_MODULES_JSON "")

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)
