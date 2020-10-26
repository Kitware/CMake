cmake_minimum_required(VERSION 3.18)
project(MixedStandardLevels3 CXX CUDA)

add_executable(MixedStandardLevels3 main.cu lib.cpp)
target_compile_features(MixedStandardLevels3 PUBLIC cuda_std_03 cxx_std_14)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET MixedStandardLevels3 PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
