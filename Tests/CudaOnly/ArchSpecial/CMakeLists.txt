cmake_minimum_required(VERSION 3.20)
project(ArchSpecial CUDA)

if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA" AND
   CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
  set(compile_options -Wno-deprecated-gpu-targets)
endif()

function(verify_output flag)
  string(REPLACE "-" "_" architectures "${flag}")
  string(TOUPPER "${architectures}" architectures)
  set(architectures "${CMAKE_CUDA_ARCHITECTURES_${architectures}}")
  list(TRANSFORM architectures REPLACE "-real" "")

  if(CMAKE_CUDA_COMPILER_ID STREQUAL "Clang")
    set(match_regex "-target-cpu sm_([0-9]+)")
  elseif(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
    set(match_regex "-arch compute_([0-9]+)")
  endif()

  string(REGEX MATCHALL "${match_regex}" target_cpus "${output}")

  foreach(cpu ${target_cpus})
    string(REGEX MATCH "${match_regex}" dont_care "${cpu}")
    list(APPEND command_archs "${CMAKE_MATCH_1}")
  endforeach()

  list(SORT command_archs)
  list(REMOVE_DUPLICATES command_archs)
  if(NOT "${command_archs}" STREQUAL "${architectures}")
    message(FATAL_ERROR "Architectures used for \"${flag}\" don't match the reference (\"${command_archs}\" != \"${architectures}\").")
  endif()
endfunction()

set(try_compile_flags -v ${compile_options})

set(CMAKE_CUDA_ARCHITECTURES all)
try_compile(all_archs_compiles
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile/all_archs_compiles
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cu
  COMPILE_DEFINITIONS ${try_compile_flags}
  OUTPUT_VARIABLE output
  )
verify_output(all)

set(CMAKE_CUDA_ARCHITECTURES all-major)
try_compile(all_major_archs_compiles
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile/all_major_archs_compiles
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cu
  COMPILE_DEFINITIONS ${try_compile_flags}
  OUTPUT_VARIABLE output
  )
verify_output(all-major)

set(CMAKE_CUDA_ARCHITECTURES native)
try_compile(native_archs_compiles
  ${CMAKE_CURRENT_BINARY_DIR}/try_compile/native_archs_compiles
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cu
  COMPILE_DEFINITIONS ${try_compile_flags}
  OUTPUT_VARIABLE output
  )
verify_output(native)

if(all_archs_compiles AND all_major_archs_compiles AND native_archs_compiles)
  set(CMAKE_CUDA_ARCHITECTURES all)
  add_executable(CudaOnlyArchSpecial main.cu)
  target_compile_options(CudaOnlyArchSpecial PRIVATE ${compile_options})
endif()
