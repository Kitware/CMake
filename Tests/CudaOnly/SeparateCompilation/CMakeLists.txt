
cmake_minimum_required(VERSION 3.7)
project (CudaOnlySeparateCompilation CUDA)

#Goal for this example:
#Build a static library that defines multiple methods and kernels that
#use each other.
#After that confirm that we can call those methods from dynamic libraries
#and executables.
#We complicate the matter by also testing that multiple static libraries
#all containing cuda separable compilation code links properly
string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_30,code=compute_30")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CUDA_STANDARD 11)
add_library(CUDASerarateLibA STATIC file1.cu file2.cu file3.cu)

#Having file4/file5 in a shared library causes serious problems
#with the nvcc linker and it will generate bad entries that will
#cause a segv when trying to run the executable
#
add_library(CUDASerarateLibB STATIC file4.cu file5.cu)
target_link_libraries(CUDASerarateLibB PRIVATE CUDASerarateLibA)

add_executable(CudaOnlySeparateCompilation main.cu)
target_link_libraries(CudaOnlySeparateCompilation PRIVATE CUDASerarateLibB)

set_target_properties( CUDASerarateLibA
                       CUDASerarateLibB
                       PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

set_target_properties( CUDASerarateLibA
                       CUDASerarateLibB
                       PROPERTIES POSITION_INDEPENDENT_CODE ON)
