cmake_minimum_required(VERSION 3.24)
project(cxx_modules_public_req_private CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(public_req_private)
target_sources(public_req_private
  PRIVATE
    FILE_SET private TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        priv.cxx
  PUBLIC
    FILE_SET public TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
      FILES
        pub.cxx)
target_compile_features(public_req_private PUBLIC cxx_std_20)

add_test(NAME cmake-version COMMAND "${CMAKE_COMMAND}" --version)
