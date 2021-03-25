cmake_minimum_required(VERSION 3.15)
project(CudaOnlyToolkitBeforeLang CXX)

# Validate that CUDAToolkit gets the correct version
# when called before CUDA the language is enabled
find_package(CUDAToolkit REQUIRED)
enable_language(CUDA)

if(NOT DEFINED CUDAToolkit_VERSION)
  message(FATAL_ERROR "expected CUDAToolkit variable CUDAToolkit_VERSION not found")
endif()

set(cuda_libs cudart cuda_driver)

# Verify that all the CUDA:: targets and variables exist
foreach (cuda_lib IN LISTS cuda_libs)
  if(NOT CUDA_${cuda_lib}_LIBRARY)
    message(FATAL_ERROR "expected CUDAToolkit variable CUDA_${cuda_lib}_LIBRARY not found")
  endif()
  if(NOT TARGET CUDA::${cuda_lib})
    message(FATAL_ERROR "expected CUDAToolkit target CUDA::${cuda_lib} not found")
  endif()
endforeach()

add_executable(CudaOnlyToolkitBeforeLang main.cu)
target_link_libraries(CudaOnlyToolkitBeforeLang PRIVATE CUDA::toolkit)
