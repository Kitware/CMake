cmake_minimum_required(VERSION 3.25)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(TryCompileTargetStatic LANGUAGES CUDA)

add_executable(CudaOnlyTryCompileTargetStatic main.cu)
