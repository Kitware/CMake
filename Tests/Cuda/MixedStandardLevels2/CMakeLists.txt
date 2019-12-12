cmake_minimum_required(VERSION 3.7)
project(MixedStandardLevels2 CXX CUDA)

string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_30,code=compute_30")

set(CMAKE_CXX_STANDARD 17) #this can decay

add_executable(MixedStandardLevels2 main.cu lib.cpp)
target_compile_features(MixedStandardLevels2 PUBLIC cuda_std_11)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET MixedStandardLevels2 PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
