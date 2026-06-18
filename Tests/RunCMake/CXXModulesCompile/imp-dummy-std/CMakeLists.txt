set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
set(CMAKE_CXX_STDLIB_MODULES_JSON
  "${CMAKE_CURRENT_LIST_DIR}/../dummy-std/manifest/dummy-std.modules.json"
)

cmake_minimum_required(VERSION 3.29)
project(cxx_modules_import_std CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

set(CMAKE_CXX_MODULE_STD 1)

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)

add_test(NAME main COMMAND main)
