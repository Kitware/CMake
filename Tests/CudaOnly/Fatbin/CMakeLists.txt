cmake_minimum_required(VERSION 3.18)
project(CudaFATBIN LANGUAGES CUDA)


set(CMAKE_CUDA_ARCHITECTURES all-major)

add_library(CudaFATBIN OBJECT
${CMAKE_CURRENT_SOURCE_DIR}/../CUBIN/kernelA.cu
${CMAKE_CURRENT_SOURCE_DIR}/../CUBIN/kernelB.cu
${CMAKE_CURRENT_SOURCE_DIR}/../CUBIN/kernelC.cu)

set_property(TARGET CudaFATBIN PROPERTY CUDA_FATBIN_COMPILATION ON)

# Will use `cuModuleLoadFatBinary` to load the fatbinaries
add_executable(CudaOnlyFatbin main.cu)
target_compile_features(CudaOnlyFatbin PRIVATE cuda_std_11)
target_compile_definitions(CudaOnlyFatbin PRIVATE "FATBIN_FILE_PATHS=\"$<JOIN:$<TARGET_OBJECTS:CudaFATBIN>,~_~>\"")

find_package(CUDAToolkit REQUIRED)
target_link_libraries(CudaOnlyFatbin PRIVATE CUDA::cuda_driver)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyFatbin PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
