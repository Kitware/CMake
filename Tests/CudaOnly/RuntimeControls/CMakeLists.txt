cmake_minimum_required(VERSION 3.7)
project (RuntimeControls CUDA)

# Find nm and dumpbin
if(CMAKE_NM)
  set(dump_command ${CMAKE_NM})
  set(dump_args -g)
else()
  include(GetPrerequisites)
  message(STATUS "calling list_prerequisites to find dumpbin")
  list_prerequisites("${CMAKE_COMMAND}" 0 0 0)
  if(gp_dumpbin)
    set(dump_command ${gp_dumpbin})
    set(dump_args /ARCHIVEMEMBERS)
  endif()
endif()

string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_30,code=[compute_30]")

set(CMAKE_CUDA_STANDARD 11)
set(CMAKE_CUDA_RUNTIME_LIBRARY static)

if(NOT "x${CMAKE_CUDA_SIMULATE_ID}" STREQUAL "xMSVC")
  add_library(UsesNoCudaRT SHARED file1.cu)
  set_target_properties(UsesNoCudaRT PROPERTIES CUDA_RUNTIME_LIBRARY none)
endif()

add_library(UsesStaticCudaRT SHARED file2.cu)

add_executable(CudaOnlyRuntimeControls main.cu)
set_target_properties(CudaOnlyRuntimeControls PROPERTIES CUDA_RUNTIME_LIBRARY shared)

target_link_libraries(CudaOnlyRuntimeControls PRIVATE $<TARGET_NAME_IF_EXISTS:UsesNoCudaRT> UsesStaticCudaRT)


if(dump_command)
  if(TARGET UsesNoCudaRT)
    add_custom_command(TARGET UsesNoCudaRT POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -DDUMP_COMMAND=${dump_command}
    -DDUMP_ARGS=${dump_args}
    -DTEST_LIBRARY_PATH=$<TARGET_FILE:UsesNoCudaRT>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/no_runtime.cmake
    )
  endif()
  add_custom_command(TARGET UsesStaticCudaRT POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -DDUMP_COMMAND=${dump_command}
    -DDUMP_ARGS=${dump_args}
    -DTEST_LIBRARY_PATH=$<TARGET_FILE:UsesStaticCudaRT>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/uses_static_runtime.cmake
  )
  string(REPLACE ";" "|" dirs "${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES}")
  add_custom_command(TARGET CudaOnlyRuntimeControls POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -DEXEC_PATH=$<TARGET_FILE:CudaOnlyRuntimeControls>
    -DEXTRA_LIB_DIRS="${dirs}"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/verify_runtime.cmake
  )
endif()
