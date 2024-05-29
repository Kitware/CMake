cmake_minimum_required(VERSION 3.15)
project(ToolkitBeforeLang CXX)

#Goal for this example:
# Validate that we use the same CUDA Toolkit as `enable_language(CUDA)` would find
find_package(CUDAToolkit REQUIRED)

if(NOT DEFINED CUDAToolkit_VERSION)
  message(FATAL_ERROR "expected CUDAToolkit variable CUDAToolkit_VERSION not found")
endif()

enable_language(CUDA)

message(STATUS "CUDAToolkit_NVCC_EXECUTABLE ${CUDAToolkit_NVCC_EXECUTABLE}")
message(STATUS "CMAKE_CUDA_COMPILER ${CMAKE_CUDA_COMPILER}")

if( NOT CUDAToolkit_NVCC_EXECUTABLE STREQUAL CMAKE_CUDA_COMPILER)
  message(FATAL_ERROR "CUDAToolkit_NVCC_EXECUTABLE ${CUDAToolkit_NVCC_EXECUTABLE} doesn't match CMAKE_CUDA_COMPILER ${CMAKE_CUDA_COMPILER}")
endif()

add_executable(ToolkitBeforeLang main.cpp)
target_link_libraries(ToolkitBeforeLang PRIVATE CUDA::toolkit)
