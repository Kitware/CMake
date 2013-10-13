cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0025 NEW)
project(target_compile_features)

set(CMAKE_VERBOSE_MAKEFILE ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU
    OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  add_executable(gnuxx_typeof_test gnuxx_typeof_test.cpp)
  target_compile_features(gnuxx_typeof_test PRIVATE gnuxx_typeof)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  add_executable(msvcxx_sealed_test msvcxx_sealed_test.cpp)
  target_compile_features(msvcxx_sealed_test PRIVATE msvcxx_sealed)
endif()

if (NOT ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_delegating_constructors;")
  add_executable(target_compile_features dummy.cpp)
  return()
endif()

add_executable(target_compile_features main.cpp)
target_compile_features(target_compile_features
  PRIVATE cxx_delegating_constructors
)

add_library(lib_delegating_constructors lib_delegating_constructors.cpp)
target_compile_features(lib_delegating_constructors
  PUBLIC cxx_delegating_constructors
)

add_executable(lib_user lib_user.cpp)
target_link_libraries(lib_user lib_delegating_constructors)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
  add_executable(binary_literals binary_literals.cpp)
  target_compile_features(binary_literals
    PRIVATE cxx_binary_literals
  )
endif()

add_subdirectory(feature_conditional_link)
