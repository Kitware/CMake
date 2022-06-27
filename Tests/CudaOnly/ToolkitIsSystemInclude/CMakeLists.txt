cmake_minimum_required(VERSION 3.24)
project(ToolkitIsSystemInclude CUDA)

# Verify that the nvrtc.h that is inside `CMAKE_CURRENT_SOURCE_DIR` is still
# the first include for `.cu` files.
add_executable(CudaOnlySystemInclude main.cu)
target_include_directories(CudaOnlySystemInclude SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
