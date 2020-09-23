cmake_minimum_required(VERSION 3.18)
project(CudaOnlyStandard98 CUDA)

# Support setting CUDA Standard to 98 which internally gets transformed to
# CUDA03
set(CMAKE_CUDA_STANDARD 98)

add_executable(CudaOnlyStandard98 main.cu)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyStandard98 PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
