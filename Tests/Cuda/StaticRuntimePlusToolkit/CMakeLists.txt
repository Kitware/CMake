cmake_minimum_required(VERSION 3.15)
project(StaticRuntimePlusToolkit CXX)

#Goal for this example:
# Validate that with c++ we can use some components of the CUDA toolkit, and
# specify the cuda runtime
find_package(CUDAToolkit REQUIRED)

add_library(Common OBJECT curand.cpp nppif.cpp)
target_link_libraries(Common PRIVATE CUDA::toolkit)
set_target_properties(Common PROPERTIES POSITION_INDEPENDENT_CODE ON)

#static runtime with shared toolkit libraries
add_library(SharedToolkit SHARED shared.cpp)
target_link_libraries(SharedToolkit PRIVATE Common PUBLIC CUDA::curand CUDA::nppif)
target_link_libraries(SharedToolkit PUBLIC CUDA::cudart_static)

#static runtime with static toolkit libraries
add_library(StaticToolkit SHARED static.cpp)
target_link_libraries(StaticToolkit PRIVATE Common CUDA::curand_static CUDA::nppif_static)
target_link_libraries(StaticToolkit PUBLIC CUDA::cudart_static)

#static runtime with mixed toolkit libraries
add_library(MixedToolkit SHARED mixed.cpp)
target_link_libraries(MixedToolkit PRIVATE Common CUDA::curand CUDA::nppif_static)
target_link_libraries(MixedToolkit PUBLIC CUDA::cudart_static)

add_executable(StaticRuntimePlusToolkit main.cpp)
target_link_libraries(StaticRuntimePlusToolkit PRIVATE SharedToolkit StaticToolkit MixedToolkit)
