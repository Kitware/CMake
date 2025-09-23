cmake_minimum_required(VERSION 3.18)
project(Architecture CUDA)

if(CMake_TEST_CUDA_ARCH)
  set(arch ${CMake_TEST_CUDA_ARCH})
else()
  set(arch 52)
endif()
add_compile_definitions(EXPECT_CUDA_ARCH=${arch})

add_executable(Architecture main.cu)
set_property(TARGET Architecture PROPERTY CUDA_ARCHITECTURES ${arch})

# Make sure CMake doesn't pass architectures if CUDA_ARCHITECTURES is OFF.
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -arch=sm_${arch}")
elseif(CMAKE_CUDA_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --cuda-gpu-arch=sm_${arch}")
endif()

add_executable(ArchitectureOff main.cu)
set_property(TARGET ArchitectureOff PROPERTY CUDA_ARCHITECTURES OFF)
