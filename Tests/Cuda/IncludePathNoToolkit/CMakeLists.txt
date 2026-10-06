cmake_minimum_required(VERSION 3.10)
project (IncludePathNoToolkit CXX CUDA)

#Goal for this example:
# Validate that between the CXX implicit include directories and the
# CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES directories we can find
# the cuda runtime headers

add_executable(IncludePathNoToolkit main.cpp)
if(CMAKE_CUDA_COMPILER_TOOLKIT_VERSION VERSION_GREATER_EQUAL 13)
  target_compile_features(IncludePathNoToolkit PRIVATE cxx_std_17)
endif()
target_include_directories(IncludePathNoToolkit PRIVATE
                           ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
