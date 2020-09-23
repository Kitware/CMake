cmake_minimum_required(VERSION 3.18)
project(CXXStandardSetTwice CXX CUDA)

set(CMAKE_CXX_STANDARD 11)

add_executable(CXXStandardSetTwice main.cu)
target_compile_features(CXXStandardSetTwice PUBLIC cxx_std_11)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CXXStandardSetTwice PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
