cmake_minimum_required(VERSION 3.15)
project(Toolkit CXX)

#Goal for this example:
# Validate that we can use CUDAToolkit to find cuda include paths
find_package(CUDAToolkit REQUIRED)

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

set(cuda_libs cudart cuda_driver cublas cufft cufftw curand cusolver cusparse)
if(CUDAToolkit_VERSION VERSION_GREATER_EQUAL 10.1)
  list(APPEND cuda_libs cublasLt)
endif()
if(CUDAToolkit_VERSION_MAJOR VERSION_LESS 11)
  list(APPEND cuda_libs nvgraph)
endif()

# Verify that all the CUDA:: targets exist even when the CUDA language isn't enabled
foreach (cuda_lib IN LISTS cuda_libs)
  if(NOT CUDA_${cuda_lib}_LIBRARY)
    message(FATAL_ERROR "expected CUDAToolkit variable CUDA_${cuda_lib}_LIBRARY not found")
  endif()
  if(NOT TARGET CUDA::${cuda_lib})
    message(FATAL_ERROR "expected CUDAToolkit target CUDA::${cuda_lib} not found")
  endif()
endforeach()

set(npp_libs nppc nppial nppicc nppidei nppif nppig nppim nppist nppitc npps nppisu)
if(CUDAToolkit_VERSION_MAJOR VERSION_LESS 11)
  list(APPEND npp_libs nppicom)
endif()
foreach (cuda_lib IN LISTS npp_libs)
  if(NOT TARGET CUDA::${cuda_lib})
    message(FATAL_ERROR "The CUDA::${cuda_lib} target was expected but couldn't be found")
  endif()
endforeach()

foreach (cuda_lib nvrtc nvToolsExt OpenCL)
  if(NOT TARGET CUDA::${cuda_lib})
    message(FATAL_ERROR "The CUDA::${cuda_lib} target was expected but couldn't be found")
  endif()
endforeach()

add_executable(Toolkit main.cpp)
target_link_libraries(Toolkit PRIVATE CUDA::toolkit)

# cupti is an optional component of the CUDA toolkit
if(TARGET CUDA::cupti)
  add_executable(cupti cupti.cpp)
  target_link_libraries(cupti PRIVATE CUDA::cupti)
endif()
