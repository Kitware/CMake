set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "d0edc3af-4c50-42ea-a356-e2862fe7a444")

cmake_minimum_required(VERSION 3.29)
project(cxx_modules_import_std CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_CXX_MODULE_STD 1)

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)

add_test(NAME main COMMAND main)
