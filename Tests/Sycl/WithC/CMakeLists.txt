cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(WithC SYCL C)
add_executable(SyclWithC main.c sycl.sycl)
