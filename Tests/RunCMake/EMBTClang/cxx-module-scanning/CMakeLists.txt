cmake_minimum_required(VERSION 3.28)
project(scantest CXX)

# CMP0155 is NEW, so a C++20 target would be scanned for module dependencies
# if the toolchain provided a scanning rule.
add_library(scantest STATIC scantest.cpp)
target_compile_features(scantest PRIVATE cxx_std_20)

if(WITH_MODULE)
  add_library(modtest STATIC)
  target_sources(modtest PUBLIC FILE_SET CXX_MODULES FILES modtest.cppm)
  target_compile_features(modtest PUBLIC cxx_std_20)
endif()
