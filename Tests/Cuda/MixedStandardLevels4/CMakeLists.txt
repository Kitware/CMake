cmake_minimum_required(VERSION 3.7)
project(MixedStandardLevels4 CXX CUDA)

string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_30,code=compute_30")

set(CMAKE_CUDA_STANDARD 03)

add_executable(MixedStandardLevels4 main.cu lib.cpp)
target_compile_features(MixedStandardLevels4 PUBLIC cxx_std_14)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET MixedStandardLevels4 PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
