cmake_minimum_required(VERSION 3.24)
project(cxx_modules_generated CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx.in"
  COMMAND "${CMAKE_COMMAND}"
          -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx.in"
          "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx"
  COMMENT "Copying 'importable.cxx'")

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
