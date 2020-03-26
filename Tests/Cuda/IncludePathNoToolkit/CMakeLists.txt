cmake_minimum_required(VERSION 3.8)
project (IncludePathNoToolkit CXX CUDA)

#Goal for this example:
# Validate that between the CXX implicit include directories and the
# CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES directories we can find
# the cuda runtime headers

add_executable(IncludePathNoToolkit main.cpp)
target_include_directories(IncludePathNoToolkit PRIVATE
                           ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
