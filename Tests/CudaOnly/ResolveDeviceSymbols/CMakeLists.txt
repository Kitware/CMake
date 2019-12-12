cmake_minimum_required(VERSION 3.7)
project (ResolveDeviceSymbols CUDA)

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

#Goal for this example:
# 1. Build two static libraries that defines multiple methods and kernels
# 2. Resolve the device symbols into the second static library, therefore
# confirming that the first static library is on the device link line
# 3. Verify that we can't use those device symbols from anything that links
# to the static library
string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_30,code=[sm_30] -gencode arch=compute_50,code=\\\"compute_50\\\"")

add_library(CUDAResolveDeviceDepsA STATIC file1.cu)
add_library(CUDAResolveDeviceDepsB STATIC file2.cu)
target_compile_features(CUDAResolveDeviceDepsA PUBLIC cuda_std_11)
target_compile_features(CUDAResolveDeviceDepsB PUBLIC cuda_std_11)
set_target_properties(CUDAResolveDeviceDepsA CUDAResolveDeviceDepsB
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON
                      POSITION_INDEPENDENT_CODE ON)

add_library(CUDAResolveDeviceLib STATIC file2_launch.cu)
set_target_properties(CUDAResolveDeviceLib
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON
                      CUDA_RESOLVE_DEVICE_SYMBOLS ON
                      POSITION_INDEPENDENT_CODE ON)
target_link_libraries(CUDAResolveDeviceLib PRIVATE CUDAResolveDeviceDepsA CUDAResolveDeviceDepsB)

if(dump_command)
add_custom_command(TARGET CUDAResolveDeviceLib POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  -DDUMP_COMMAND=${dump_command}
  -DDUMP_ARGS=${dump_args}
  -DTEST_LIBRARY_PATH=$<TARGET_FILE:CUDAResolveDeviceLib>
  -P ${CMAKE_CURRENT_SOURCE_DIR}/verify.cmake
  )
endif()

add_executable(CudaOnlyResolveDeviceSymbols main.cu)
set_target_properties(CudaOnlyResolveDeviceSymbols
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION OFF
                      CUDA_RESOLVE_DEVICE_SYMBOLS OFF)

target_link_libraries(CudaOnlyResolveDeviceSymbols PRIVATE CUDAResolveDeviceLib)

if(APPLE)
  # Help the static cuda runtime find the driver (libcuda.dyllib) at runtime.
  set_property(TARGET CudaOnlyResolveDeviceSymbols PROPERTY BUILD_RPATH ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
endif()
