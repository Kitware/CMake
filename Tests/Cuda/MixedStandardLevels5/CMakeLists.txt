cmake_minimum_required(VERSION 3.7)
project(MixedStandardLevels5 CXX CUDA)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CUDA_ARCHITECTURES 30)

add_executable(MixedStandardLevels5 main.cu lib.cpp)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET MixedStandardLevels5 PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
