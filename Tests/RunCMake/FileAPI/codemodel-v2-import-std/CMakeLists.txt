set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

cmake_minimum_required(VERSION 3.29)
project(codemodel_v2_import_std CXX)

cmake_file_api(QUERY API_VERSION 1 CODEMODEL 2)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../CXXModulesCompile/cxx-modules-rules.cmake")

set(CMAKE_CXX_MODULE_STD 1)

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)
