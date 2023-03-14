cmake_minimum_required(VERSION 3.18)
project(CudaOptix LANGUAGES CUDA)


set(CMAKE_CUDA_ARCHITECTURES all-major)

add_library(CudaOptix OBJECT
  ${CMAKE_CURRENT_SOURCE_DIR}/../CUBIN/kernelA.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/../CUBIN/kernelB.cu
  ${CMAKE_CURRENT_SOURCE_DIR}/../CUBIN/kernelC.cu)

if(CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL "11.7.0")
  set_property(TARGET CudaOptix PROPERTY CUDA_OPTIX_COMPILATION ON)
endif()

set_property(TARGET CudaOptix PROPERTY CUDA_ARCHITECTURES native)

add_executable(CudaOnlyOptixIR main.cu)
target_compile_features(CudaOnlyOptixIR PRIVATE cuda_std_11)

if(CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL "11.7.0")
  target_compile_definitions(CudaOnlyOptixIR PRIVATE "OPTIX_FILE_PATHS=\"$<JOIN:$<TARGET_OBJECTS:CudaOptix>,~_~>\"")
else()
  target_compile_definitions(CudaOnlyOptixIR PRIVATE "OPTIX_FILE_PATHS=\"NO_OPTIX_SUPPORT\"")
endif()

find_package(CUDAToolkit REQUIRED)
target_link_libraries(CudaOnlyOptixIR PRIVATE CUDA::cuda_driver)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyOptixIR PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
