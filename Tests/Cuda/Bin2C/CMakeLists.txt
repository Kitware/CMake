cmake_minimum_required(VERSION 4.2)
project(Bin2C CXX)

# Goal for this example:
# Validate that we can use CUDAToolkit bin2c
find_package(CUDAToolkit REQUIRED)

add_executable(generate generate.cpp)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/binary.bin
  COMMAND generate ${CMAKE_CURRENT_BINARY_DIR}/binary.bin
  )
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/binary.h
  COMMAND CUDA::bin2c ${CMAKE_CURRENT_BINARY_DIR}/binary.bin > ${CMAKE_CURRENT_BINARY_DIR}/binary.h
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/binary.bin
  )
add_executable(CudaBin2C verify.cpp ${CMAKE_CURRENT_BINARY_DIR}/binary.h)
target_include_directories(CudaBin2C PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
