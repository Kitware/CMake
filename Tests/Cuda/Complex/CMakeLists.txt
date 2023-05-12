cmake_minimum_required(VERSION 3.18)
project(Complex CXX CUDA)
#Goal for this example:

#build a cpp dynamic library base
#build a cuda static library base that uses separable compilation

#build a cuda dynamic library that uses the first dynamic library
#build a mixed cpp & cuda dynamic library uses all 3 previous libraries

#lastly build a cpp executable that uses this last cuda dynamic library

#this tests that we can properly handle linking cuda and cpp together
#and also building cpp targets that need cuda implicit libraries

set(CMAKE_CUDA_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(CudaComplexCppBase SHARED dynamic.cpp)
add_library(CudaComplexSharedLib SHARED dynamic.cu)
target_link_libraries(CudaComplexSharedLib PUBLIC CudaComplexCppBase)

add_library(CudaComplexSeperableLib STATIC file1.cu file2.cu file3.cu)
add_library(CudaComplexMixedLib SHARED mixed.cpp mixed.cu)
target_link_libraries(CudaComplexMixedLib
                      PUBLIC CudaComplexSharedLib
                      PRIVATE CudaComplexSeperableLib)

add_executable(CudaComplex main.cpp)
target_link_libraries(CudaComplex PUBLIC CudaComplexMixedLib)


set_target_properties(CudaComplexMixedLib
                      CudaComplexSeperableLib
                      PROPERTIES
                        POSITION_INDEPENDENT_CODE  ON
                        CUDA_SEPARABLE_COMPILATION ON
                      )
set_target_properties(CudaComplexMixedLib
                      CudaComplexSharedLib
                      PROPERTIES
                        CUDA_RUNTIME_LIBRARY shared
                      )


if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaComplex PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()

if(UNIX)
  # Help the shared cuda runtime find libcudart as it is not located
  # in a default system searched location
  find_package(CUDAToolkit REQUIRED)
  set_property(TARGET CudaComplexMixedLib PROPERTY BUILD_RPATH "${CUDAToolkit_LIBRARY_DIR}")
endif()
