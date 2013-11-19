cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0025 NEW)
project(compile_features)

if (EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.cxx")
  file(READ "${CMAKE_BINARY_DIR}/CMakeFiles/feature_tests.cxx" featureTest)
  message("_CMAKE_CXX_CREATE_OBJECT_FILE: ${_CMAKE_CXX_CREATE_OBJECT_FILE}")
  message("_compile_object_command: ${_compile_object_command}")
  message("CTEST_FULL_OUTPUT\n${featureTest}")
else()
  message("No known features!")
endif()

add_executable(compile_features main.cpp)
