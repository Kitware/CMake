cmake_minimum_required(VERSION 3.29)
project(cxx_modules_import_std_no_std_property CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_CXX_MODULE_STD 0)

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)
