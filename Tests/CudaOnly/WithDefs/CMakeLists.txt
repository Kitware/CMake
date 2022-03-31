cmake_minimum_required(VERSION 3.18)
project(WithDefs CUDA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(release_compile_defs DEFREL)

#Goal for this example:
#build a executable that needs to be passed a complex define through add_defintions
#this verifies we can pass things such as '_','(' to nvcc
add_definitions("-DPACKED_DEFINE=__attribute__((packed))")

add_executable(CudaOnlyWithDefs main.notcu)
set_source_files_properties(main.notcu PROPERTIES LANGUAGE CUDA)

target_compile_options(CudaOnlyWithDefs
  PRIVATE
    -DFLAG_COMPILE_LANG_$<COMPILE_LANGUAGE>
    -DFLAG_LANG_IS_CUDA=$<COMPILE_LANGUAGE:CUDA>
    $<$<CUDA_COMPILER_ID:NVIDIA>:--compiler-options=-DHOST_DEFINE> # Host-only defines are possible only on NVCC.
  )

target_compile_definitions(CudaOnlyWithDefs
  PRIVATE
    $<$<CONFIG:RELEASE>:$<BUILD_INTERFACE:${release_compile_defs}>>
    -DDEF_COMPILE_LANG_$<COMPILE_LANGUAGE>
    -DDEF_LANG_IS_CUDA=$<COMPILE_LANGUAGE:CUDA>
    -DDEF_CUDA_COMPILER=$<CUDA_COMPILER_ID>
    -DDEF_CUDA_COMPILER_VERSION=$<CUDA_COMPILER_VERSION>
  )

target_include_directories(CudaOnlyWithDefs
  PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:${CMAKE_CURRENT_SOURCE_DIR}/inc_cuda>
)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyWithDefs PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
