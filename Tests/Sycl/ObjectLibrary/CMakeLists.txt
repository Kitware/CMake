cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(ObjectLibrary SYCL CXX)

add_subdirectory(Conflicts)
add_library(SyclMixedObjectLib OBJECT static.sycl static.cpp)
add_executable(SyclObjectLibrary main.cpp
  $<TARGET_OBJECTS:SyclMixedObjectLib> $<TARGET_OBJECTS:SyclConflicts>)
