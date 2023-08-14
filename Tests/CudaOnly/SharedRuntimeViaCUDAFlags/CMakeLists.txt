cmake_minimum_required(VERSION 3.17)
project(SharedRuntimeViaCUDAFlags NONE)

set(CMAKE_CUDA_FLAGS "")
string(APPEND CMAKE_CUDA_FLAGS "-cudart shared")

enable_language(CUDA)

add_executable(CudaOnlySharedRuntimeViaCUDAFlags main.cu)

if(UNIX)
  # Help the shared cuda runtime find libcudart as it is not located
  # in a default system searched location
  find_package(CUDAToolkit REQUIRED)
  set_property(TARGET CudaOnlySharedRuntimeViaCUDAFlags PROPERTY BUILD_RPATH "${CUDAToolkit_LIBRARY_DIR}")
endif()
