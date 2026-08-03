set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "25d6f6aa-be65-4692-b44e-87b23e96d4e1")

cmake_minimum_required(VERSION 3.29)
project(codemodel_v2_import_std CXX)

cmake_file_api(QUERY API_VERSION 1 CODEMODEL 2)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../CXXModulesCompile/cxx-modules-rules.cmake")

add_executable(main
  main.cxx)
target_compile_features(main PRIVATE cxx_std_23)
