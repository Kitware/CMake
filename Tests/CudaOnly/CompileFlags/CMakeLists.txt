cmake_minimum_required(VERSION 3.17)
project(CompileFlags CUDA)

if(CMake_TEST_CUDA_ARCH)
  set(arch ${CMake_TEST_CUDA_ARCH})
else()
  set(arch 52)
endif()

add_executable(CudaOnlyCompileFlags main.cu)
target_compile_definitions(CudaOnlyCompileFlags PRIVATE EXPECT_CUDA_ARCH=${arch})

# Try passing CUDA architecture flags explicitly.
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  target_compile_options(CudaOnlyCompileFlags PRIVATE
    -gencode arch=compute_${arch},code=compute_${arch}
  )
  set_property(TARGET CudaOnlyCompileFlags PROPERTY CUDA_ARCHITECTURES)
else()
  set_property(TARGET CudaOnlyCompileFlags PROPERTY CUDA_ARCHITECTURES ${arch}-real)
endif()

target_compile_options(CudaOnlyCompileFlags PRIVATE -DALWAYS_DEFINE)
