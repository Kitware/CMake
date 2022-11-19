# Set CMAKE_CUDA_SEPARABLE_COMPILATION before `project`
# so we verify that compiler/linker verbose extraction
# works as required when a `dlink` is part of it
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
project(SeparableCompCXXOnly LANGUAGES CXX CUDA)
add_executable(SeparableCompCXXOnly main.cpp)
