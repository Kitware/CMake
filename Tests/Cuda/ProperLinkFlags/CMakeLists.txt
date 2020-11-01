cmake_minimum_required(VERSION 3.18)
project(ProperLinkFlags CUDA CXX)

#Goal for this example:
#Verify that when we have CXX and CUDA enabled and we link an executable that
#has CUDA and CXX we use the CUDA link flags when doing the device link
#step

#Specify a set of valid CUDA flags and an invalid set of CXX flags ( for CUDA )
#to make sure we don't use the CXX flags when linking CUDA executables
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  string(APPEND CMAKE_CUDA_FLAGS "--use_fast_math")
elseif(CMAKE_CUDA_COMPILER_ID STREQUAL "Clang")
  string(APPEND CMAKE_CUDA_FLAGS "-ffast-math")
endif()

set(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CUDA_STANDARD 11)
add_executable(ProperLinkFlags file1.cu main.cxx)

set_target_properties( ProperLinkFlags
                       PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
