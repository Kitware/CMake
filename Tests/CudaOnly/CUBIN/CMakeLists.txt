cmake_minimum_required(VERSION 3.18)
unset(ENV{CMAKE_CUDA_ARCHITECTURES_NATIVE_CLAMP}) # CUBIN needs true native arch
project(CudaCUBIN LANGUAGES CUDA)

set(CMAKE_CUDA_ARCHITECTURES all-major)

# CUBIN needs the true native arch to be supported by the CUDA toolkit.
set(unavailable_native_archs "${CMAKE_CUDA_ARCHITECTURES_NATIVE}")
list(REMOVE_ITEM unavailable_native_archs ${CMAKE_CUDA_ARCHITECTURES_ALL})
if(unavailable_native_archs)
  add_executable(CudaOnlyCUBIN main_no_native_archs.cu)
  return()
endif()

add_library(CudaCUBIN OBJECT kernelA.cu kernelB.cu kernelC.cu)
set_property(TARGET CudaCUBIN PROPERTY CUDA_CUBIN_COMPILATION ON)
set_property(TARGET CudaCUBIN PROPERTY CUDA_ARCHITECTURES native)

add_executable(CudaOnlyCUBIN main.cu)
target_compile_features(CudaOnlyCUBIN PRIVATE cuda_std_11)
target_compile_definitions(CudaOnlyCUBIN PRIVATE "CUBIN_FILE_PATHS=\"$<JOIN:$<TARGET_OBJECTS:CudaCUBIN>,~_~>\"")

find_package(CUDAToolkit REQUIRED)
target_link_libraries(CudaOnlyCUBIN PRIVATE CUDA::cuda_driver)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyCUBIN PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
