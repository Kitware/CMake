cmake_minimum_required(VERSION 3.18)
project(GPUDebugFlag CUDA)

set(CMAKE_CUDA_STANDARD 11)

# Goal for this example:
# Verify that enabling device debug works.
string(APPEND CMAKE_CUDA_FLAGS " -G")

add_executable(CudaOnlyGPUDebugFlag main.cu)

#CUDA's __CUDACC_DEBUG__ define was added in NVCC 9.0
#so if we are below 9.0.0 we will manually add the define so that the test
#passes
if(CMAKE_CUDA_COMPILER_VERSION VERSION_LESS 9.0.0)
  target_compile_definitions(CudaOnlyGPUDebugFlag PRIVATE "__CUDACC_DEBUG__")
endif()

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyGPUDebugFlag PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
