cmake_minimum_required(VERSION 3.17)
set(ENV{CUDAFLAGS} "$ENV{CUDAFLAGS} -cudart shared")
project(SharedRuntimeViaCUDAFlags CUDA)

add_executable(CudaOnlySharedRuntimeViaCUDAFlags main.cu)

if(UNIX)
  # Help the shared cuda runtime find libcudart as it is not located
  # in a default system searched location
  find_package(CUDAToolkit REQUIRED)
  set_property(TARGET CudaOnlySharedRuntimeViaCUDAFlags PROPERTY BUILD_RPATH "${CUDAToolkit_LIBRARY_DIR}")
endif()
