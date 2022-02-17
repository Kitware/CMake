cmake_minimum_required(VERSION 3.19)
project (SeparateCompilationPTX CUDA)

#Goal for this example:
# How to generate PTX files with RDC enabled

# PTX can be compiled only for a single virtual architecture at a time
list(POP_FRONT CMAKE_CUDA_ARCHITECTURES temp)
set(CMAKE_CUDA_ARCHITECTURES ${temp})
string(APPEND CMAKE_CUDA_ARCHITECTURES "-virtual")

add_library(CudaPTX OBJECT kernels.cu)
set_property(TARGET CudaPTX PROPERTY CUDA_PTX_COMPILATION ON)
set_property(TARGET CudaPTX PROPERTY CUDA_SEPARABLE_COMPILATION ON)


set(output_file ${CMAKE_CURRENT_BINARY_DIR}/embedded_objs.h)

find_package(CUDAToolkit REQUIRED)
find_program(bin_to_c
  NAMES bin2c
  PATHS ${CUDAToolkit_BIN_DIR}
  )
if(NOT bin_to_c)
  message(FATAL_ERROR
    "bin2c not found:\n"
    "  CUDAToolkit_BIN_DIR='${CUDAToolkit_BIN_DIR}'\n"
    )
endif()

add_custom_command(
  OUTPUT "${output_file}"
  COMMAND ${CMAKE_COMMAND}
    "-DBIN_TO_C_COMMAND=${bin_to_c}"
    "-DOBJECTS=$<TARGET_OBJECTS:CudaPTX>"
    "-DOUTPUT=${output_file}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../utils/bin2c_wrapper.cmake
  VERBATIM
  DEPENDS $<TARGET_OBJECTS:CudaPTX>
  COMMENT "Converting Object files to a C header"
  )

add_executable(CudaOnlySeparateCompilationPTX main.cu ${output_file})
target_compile_features(CudaOnlySeparateCompilationPTX PRIVATE cuda_std_11)
target_include_directories(CudaOnlySeparateCompilationPTX PRIVATE
                           ${CMAKE_CURRENT_BINARY_DIR} )
target_link_libraries(CudaOnlySeparateCompilationPTX PRIVATE CUDA::cuda_driver)
if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlySeparateCompilationPTX PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
