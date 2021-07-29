cmake_minimum_required(VERSION 3.18)
project(SeparateCompilation CUDA)

#Goal for this example:
#Build a static library that defines multiple methods and kernels that
#use each other.
#After that confirm that we can call those methods from dynamic libraries
#and executables.
#We complicate the matter by also testing that multiple static libraries
#all containing cuda separable compilation code links properly
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
add_library(CUDASeparateLibA STATIC file1.cu file2.cu file3.cu)
target_compile_features(CUDASeparateLibA PRIVATE cuda_std_11)
get_property(sep_comp TARGET CUDASeparateLibA PROPERTY CUDA_SEPARABLE_COMPILATION)
if(NOT sep_comp)
  message(FATAL_ERROR "CUDA_SEPARABLE_COMPILATION not initialized")
endif()
unset(CMAKE_CUDA_SEPARABLE_COMPILATION)

if(CMAKE_CUDA_SIMULATE_ID STREQUAL "MSVC")
  # Test adding a flag that is not in our CUDA flag table for VS.
  if(NOT CMAKE_CUDA_COMPILER_VERSION VERSION_LESS 8)
    string(APPEND CMAKE_CUDA_FLAGS " --ftemplate-depth 50")
  endif()
  # Test adding a flag that nvcc should pass to the host compiler.
  target_compile_options(CUDASeparateLibA PRIVATE -Xcompiler=-bigobj)
endif()

#Having file4/file5 in a shared library causes serious problems
#with the nvcc linker and it will generate bad entries that will
#cause a segv when trying to run the executable
#
add_library(CUDASeparateLibB STATIC file4.cu file5.cu)
target_compile_features(CUDASeparateLibB PRIVATE cuda_std_11)
target_link_libraries(CUDASeparateLibB PRIVATE CUDASeparateLibA)

set_target_properties(CUDASeparateLibA
                      CUDASeparateLibB
                      PROPERTIES CUDA_SEPARABLE_COMPILATION ON
                      POSITION_INDEPENDENT_CODE ON)

add_subdirectory(main)
