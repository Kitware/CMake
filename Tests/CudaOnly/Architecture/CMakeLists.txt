cmake_minimum_required(VERSION 3.18)
project(Architecture CUDA)

add_executable(Architecture main.cu)
set_property(TARGET Architecture PROPERTY CUDA_ARCHITECTURES 52)

# Make sure CMake doesn't pass architectures if CUDA_ARCHITECTURES is OFF.
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -arch=sm_52")
elseif(CMAKE_CUDA_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --cuda-gpu-arch=sm_52")
endif()

add_executable(ArchitectureOff main.cu)
set_property(TARGET ArchitectureOff PROPERTY CUDA_ARCHITECTURES OFF)
