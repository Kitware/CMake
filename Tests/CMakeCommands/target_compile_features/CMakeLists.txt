cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0025 NEW)
project(target_compile_features)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU
    OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  add_executable(gnuxx_typeof_test gnuxx_typeof_test.cpp)
endif()
if ("${CMAKE_CXX_COMPILER}" MATCHES "MSVC"
      AND NOT MSVC_VERSION VERSION_LESS 1400)
  add_executable(msvcxx_sealed_test msvcxx_sealed_test.cpp)
endif()

if (NOT ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_delegating_constructors;")
  add_executable(target_compile_features dummy.cpp)
  return()
endif()

add_executable(target_compile_features main.cpp)
target_compile_features(target_compile_features
  PRIVATE cxx_delegating_constructors
)
