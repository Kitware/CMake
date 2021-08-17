cmake_minimum_required(VERSION 3.20)
project(CudaOnlyAll CUDA)

if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA" AND
   CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 11.5.0)

  set(compile_options -Wno-deprecated-gpu-targets)
  function(verify_output flag output_var)
    string(REGEX MATCHALL "-arch compute_([0-9]+)" target_archs "${${output_var}}")
    list(LENGTH target_archs count)
    if(count LESS 2)
      message(FATAL_ERROR "${flag} failed to map to multiple architectures")
    endif()
  endfunction()
endif()

if(COMMAND verify_output)
  set(try_compile_flags -v ${compile_options})

  set(CMAKE_CUDA_ARCHITECTURES all)
  try_compile(all_archs_compiles
    ${CMAKE_CURRENT_BINARY_DIR}/try_compile/all_archs_compiles
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cu
    COMPILE_DEFINITIONS ${try_compile_flags}
    OUTPUT_VARIABLE output
    )
  verify_output(all output)

  set(CMAKE_CUDA_ARCHITECTURES all-major)
  try_compile(all_major_archs_compiles
    ${CMAKE_CURRENT_BINARY_DIR}/try_compile/all_major_archs_compiles
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cu
    COMPILE_DEFINITIONS ${try_compile_flags}
    OUTPUT_VARIABLE output
    )
  verify_output(all-major output)

  if(all_archs_compiles AND all_major_archs_compiles)
    add_executable(CudaOnlyAll main.cu)
    target_compile_options(CudaOnlyAll PRIVATE ${compile_options})
  endif()
else()
  add_executable(CudaOnlyAll main.cu)
endif()
