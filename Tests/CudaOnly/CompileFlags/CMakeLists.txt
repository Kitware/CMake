cmake_minimum_required(VERSION 3.17)
project(CompileFlags CUDA)

add_executable(CudaOnlyCompileFlags main.cu)

# Try passing CUDA architecture flags explicitly.
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  target_compile_options(CudaOnlyCompileFlags PRIVATE
    -gencode arch=compute_50,code=compute_50
  )
  set_property(TARGET CudaOnlyCompileFlags PROPERTY CUDA_ARCHITECTURES)
else()
  set_property(TARGET CudaOnlyCompileFlags PROPERTY CUDA_ARCHITECTURES 50-real)
endif()

target_compile_options(CudaOnlyCompileFlags PRIVATE -DALWAYS_DEFINE)
