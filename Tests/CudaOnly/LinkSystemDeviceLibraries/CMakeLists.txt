cmake_minimum_required(VERSION 3.8)
project(CudaOnlyLinkSystemDeviceLibraries CUDA)

string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_35,code=compute_35 -gencode arch=compute_35,code=sm_35")
set(CMAKE_CUDA_STANDARD 11)

add_executable(CudaOnlyLinkSystemDeviceLibraries main.cu)
set_target_properties( CudaOnlyLinkSystemDeviceLibraries
                       PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
target_link_libraries( CudaOnlyLinkSystemDeviceLibraries PRIVATE cublas_device)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyLinkSystemDeviceLibraries PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
