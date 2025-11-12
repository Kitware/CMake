cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_ninja_cmp0154 CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_custom_command(
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx.in"
  COMMAND "${CMAKE_COMMAND}"
          -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/importable.cxx.in"
          "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx"
  COMMENT "Copying 'importable.cxx'")

add_executable(ninja_cmp0154)
target_sources(ninja_cmp0154
  PRIVATE
    main.cxx
    unrelated.cxx
  PUBLIC
    FILE_SET CXX_MODULES
      BASE_DIRS
      "${CMAKE_CURRENT_BINARY_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/importable.cxx")
target_compile_features(ninja_cmp0154 PUBLIC cxx_std_20)
set_property(SOURCE unrelated.cxx
  PROPERTY
    CXX_SCAN_FOR_MODULES 0)

add_test(NAME ninja_cmp0154 COMMAND ninja_cmp0154)
