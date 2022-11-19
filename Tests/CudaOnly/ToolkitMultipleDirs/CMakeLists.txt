cmake_minimum_required(VERSION 3.15)
project(CudaOnlyToolkitMultipleDirs CUDA)

find_package(CUDAToolkit REQUIRED)

add_subdirectory(subdir)

if(NOT DEFINED CUDAToolkit_VERSION)
  message(FATAL_ERROR "expected CUDAToolkit variable CUDAToolkit_VERSION not found")
endif()

message(STATUS "CUDAToolkit_VERSION: ${CUDAToolkit_VERSION}")
message(STATUS "CUDAToolkit_VERSION_MAJOR: ${CUDAToolkit_VERSION_MAJOR}")
message(STATUS "CUDAToolkit_VERSION_MINOR: ${CUDAToolkit_VERSION_MINOR}")
message(STATUS "CUDAToolkit_VERSION_PATCH: ${CUDAToolkit_VERSION_PATCH}")
message(STATUS "CUDAToolkit_BIN_DIR: ${CUDAToolkit_BIN_DIR}")
message(STATUS "CUDAToolkit_INCLUDE_DIRS: ${CUDAToolkit_INCLUDE_DIRS}")
message(STATUS "CUDAToolkit_LIBRARY_DIR: ${CUDAToolkit_LIBRARY_DIR}")
message(STATUS "CUDAToolkit_NVCC_EXECUTABLE ${CUDAToolkit_NVCC_EXECUTABLE}")

add_executable(CudaOnlyToolkitMultipleDirs main.cu)
target_link_libraries(CudaOnlyToolkitMultipleDirs PRIVATE CUDA::toolkit)
