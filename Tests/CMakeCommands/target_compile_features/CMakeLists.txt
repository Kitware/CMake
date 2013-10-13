cmake_minimum_required(VERSION 2.8)

project(target_compile_features)

if (NOT ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";cxx_delegating_constructors;")
  add_executable(target_compile_features dummy.cpp)
  return()
endif()

add_executable(target_compile_features main.cpp)
target_compile_features(target_compile_features
  PRIVATE cxx_delegating_constructors
)
