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
set_target_properties(CUDASeparateLibA
                      PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)
unset(CMAKE_CUDA_SEPARABLE_COMPILATION)

if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA" AND CMAKE_CUDA_SIMULATE_ID STREQUAL "MSVC")
  # Test adding a flag that is not in our CUDA flag table for VS.
  if(NOT CMAKE_CUDA_COMPILER_VERSION VERSION_LESS 8)
    string(APPEND CMAKE_CUDA_FLAGS " --ftemplate-depth 50")
  endif()
  # Test adding a flag that nvcc should pass to the host compiler.
  target_compile_options(CUDASeparateLibA PRIVATE -Xcompiler=-bigobj)
endif()

#Have file4 and file5 in different shared libraries so that we
#verify that hidden visibility is passed to the device linker.
#Otherwise we will get a segv when trying to run the executable
#
add_library(CUDASeparateLibB SHARED file4.cu)
target_compile_features(CUDASeparateLibB PRIVATE cuda_std_11)
target_link_libraries(CUDASeparateLibB PRIVATE CUDASeparateLibA)

add_library(CUDASeparateLibC SHARED file5.cu)
target_compile_features(CUDASeparateLibC PRIVATE cuda_std_11)
target_link_libraries(CUDASeparateLibC PRIVATE CUDASeparateLibA)

set_target_properties(CUDASeparateLibB
                      CUDASeparateLibC
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON
                      POSITION_INDEPENDENT_CODE ON
                      CUDA_VISIBILITY_PRESET hidden
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/main")

add_subdirectory(main)
