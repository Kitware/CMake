cmake_minimum_required(VERSION 3.15)
project(Toolkit CXX)

#Goal for this example:
# Validate that we can use CUDAToolkit to find cuda include paths
find_package(CUDAToolkit REQUIRED)

message(STATUS "CUDAToolkit_VERSION: ${CUDAToolkit_VERSION}")
message(STATUS "CUDAToolkit_VERSION_MAJOR: ${CUDAToolkit_VERSION_MAJOR}")
message(STATUS "CUDAToolkit_VERSION_MINOR: ${CUDAToolkit_VERSION_MINOR}")
message(STATUS "CUDAToolkit_VERSION_PATCH: ${CUDAToolkit_VERSION_PATCH}")
message(STATUS "CUDAToolkit_BIN_DIR: ${CUDAToolkit_BIN_DIR}")
message(STATUS "CUDAToolkit_INCLUDE_DIRS: ${CUDAToolkit_INCLUDE_DIRS}")
message(STATUS "CUDAToolkit_LIBRARY_DIR: ${CUDAToolkit_LIBRARY_DIR}")
message(STATUS "CUDAToolkit_NVCC_EXECUTABLE ${CUDAToolkit_NVCC_EXECUTABLE}")

# Verify that all the CUDA:: targets exist even when the CUDA language isn't enabled

foreach (cuda_lib cudart cuda_driver cublas cufft cufftw curand cusolver cusparse nvgraph)
  if(NOT TARGET CUDA::${cuda_lib})
    message(FATAL_ERROR "The CUDA::${cuda_lib} target was expected but couldn't be found")
  endif()
endforeach()

foreach (cuda_lib nppc nppial nppicc nppidei nppif nppig nppim nppist nppitc npps nppicom nppisu)
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
