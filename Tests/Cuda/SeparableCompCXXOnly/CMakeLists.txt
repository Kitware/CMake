project(SeparableCompCXXOnly LANGUAGES CXX CUDA)
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
add_executable(SeparableCompCXXOnly main.cpp)
