cmake_minimum_required(VERSION 2.8)

project(target_compiler_features)

if (NOT ";${CMAKE_CXX_COMPILER_FEATURES};" MATCHES ";cxx_delegating_constructors;")
  add_executable(target_compiler_features dummy.cpp)
  return()
endif()

add_executable(target_compiler_features main.cpp)
target_compiler_features(target_compiler_features
  PRIVATE cxx_delegating_constructors
)
