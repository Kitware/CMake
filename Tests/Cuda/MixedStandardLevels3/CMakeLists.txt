cmake_minimum_required(VERSION 3.7)
project(MixedStandardLevels3 CXX CUDA)

string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_30,code=compute_30")

add_executable(MixedStandardLevels3 main.cu lib.cpp)
target_compile_features(MixedStandardLevels3 PUBLIC cuda_std_03 cxx_std_14)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET MixedStandardLevels3 PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
