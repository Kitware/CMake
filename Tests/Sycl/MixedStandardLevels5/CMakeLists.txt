cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(MixedStandardLevels5 CXX SYCL)
set(CMAKE_CXX_STANDARD 14)
# Keep SYCL at its supported baseline while CXX uses an older standard.
set(CMAKE_SYCL_CXX_STANDARD 17)
add_executable(MixedStandardLevels5 main.sycl lib.cpp)
