cmake_minimum_required(VERSION 3.18)
project(DontResolveDeviceSymbols CUDA)

# Find nm and dumpbin
if(CMAKE_NM)
  set(dump_command ${CMAKE_NM})
  set(dump_args --defined-only)
  set(symbol_name cudaRegisterLinkedBinary)
else()
  include(GetPrerequisites)
  message(STATUS "calling list_prerequisites to find dumpbin")
  list_prerequisites("${CMAKE_COMMAND}" 0 0 0)
  if(gp_dumpbin)
    set(dump_command ${gp_dumpbin})
    set(dump_args /SYMBOLS)
    set(symbol_name nv_fatb)
  endif()
endif()


#Goal for this example:
# Build a static library that defines multiple methods and kernels that
# use each other.
# Don't resolve the device symbols in the static library
# Don't resolve the device symbols in the executable library
# Verify that we can't use those device symbols from anything
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CUDA_STANDARD 11)
set(CMAKE_CUDA_RESOLVE_DEVICE_SYMBOLS OFF)

add_library(CUDANoDeviceResolve SHARED file1.cu)
set_target_properties(CUDANoDeviceResolve
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON
                      POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_link_options(CUDANoDeviceResolve PRIVATE "/FORCE:UNRESOLVED")
endif()

if(dump_command)
add_custom_command(TARGET CUDANoDeviceResolve POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  -DDUMP_COMMAND=${dump_command}
  -DDUMP_ARGS=${dump_args}
  -DSYMBOL_NAME=${symbol_name}
  -DTEST_LIBRARY_PATH=$<TARGET_FILE:CUDANoDeviceResolve>
  -P ${CMAKE_CURRENT_SOURCE_DIR}/verify.cmake
  )
endif()
