cmake_minimum_required(VERSION 3.17)
cmake_policy(SET CMP0104 OLD)
project(CompileFlags CUDA)

# Clear defaults.
set(CMAKE_CUDA_ARCHITECTURES)

add_executable(CudaOnlyCompileFlags main.cu)

# Try passing CUDA architecture flags explicitly.
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  target_compile_options(CudaOnlyCompileFlags PRIVATE
    -gencode arch=compute_50,code=compute_50
    --compiler-options=-DHOST_DEFINE
  )
endif()
