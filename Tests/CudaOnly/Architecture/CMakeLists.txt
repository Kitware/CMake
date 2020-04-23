cmake_minimum_required(VERSION 3.17)
project(Architecture CUDA)

set(CMAKE_CUDA_ARCHITECTURES 52)
add_executable(Architecture main.cu)
