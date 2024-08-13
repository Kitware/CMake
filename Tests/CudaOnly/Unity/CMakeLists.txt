cmake_minimum_required(VERSION 3.30)
project(Unity CUDA)

set(CMAKE_UNITY_BUILD 1)

add_library(UnityObjects STATIC a.cu b.cu)

add_executable(CudaOnlyUnity main.cu)
target_link_libraries(CudaOnlyUnity PRIVATE UnityObjects)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyUnity PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
