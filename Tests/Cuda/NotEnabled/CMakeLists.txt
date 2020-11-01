cmake_minimum_required(VERSION 3.7)
project(CudaNotEnabled CXX)

add_library(HasCudaProps lib.cxx)

target_compile_features(HasCudaProps PUBLIC cxx_std_11)
#Verify that setting this variables in a project that doesn't have CUDA
#enabled allow for the project to configure and build correctly.
#Tests the fix for #19432
set_property(TARGET HasCudaProps PROPERTY CUDA_SEPARABLE_COMPILATION ON)
set_property(TARGET HasCudaProps PROPERTY CUDA_RESOLVE_DEVICE_SYMBOLS ON)

add_executable(CudaNotEnabled main.cxx)
target_link_libraries(CudaNotEnabled PRIVATE HasCudaProps)
