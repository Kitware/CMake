cmake_minimum_required(VERSION 3.24)
project(cxx_modules_generated CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx"
  COPYONLY)

add_executable(generated)
target_sources(generated
  PRIVATE
    main.cxx
  PRIVATE
    FILE_SET CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx")
target_compile_features(generated PUBLIC cxx_std_20)

add_test(NAME generated COMMAND generated)
