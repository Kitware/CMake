cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(SeparateCompilation SYCL)
include(../utils/Test.cmake)
# Remove gate when IntelLLVM -fsycl-link bug is fixed
if(CMAKE_SYCL_COMPILER_ID STREQUAL "IntelLLVM")
  sycl_not_supported("IntelLLVM -fsycl-link cannot produce a single device-link output")
  return()
endif()

if(NOT CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_ON OR
   NOT CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_OFF)
  sycl_not_supported("${CMAKE_SYCL_COMPILER_ID} does not support explicit SYCL separable compilation")
  return()
endif()

# CudaOnly.SeparateCompilation's three-source static library contains external
# device functions and a kernel.  Two hidden-visibility shared libraries each
# consume it and define another kernel; the executable calls both shared images.
set(CMAKE_SYCL_SEPARABLE_COMPILATION ON)
add_library(SYCLSeparateLibA STATIC device.sycl device_extra.sycl static_kernel.sycl)
target_compile_features(SYCLSeparateLibA PRIVATE sycl_cxx_std_17)
set_property(TARGET SYCLSeparateLibA PROPERTY POSITION_INDEPENDENT_CODE ON)
get_property(separable TARGET SYCLSeparateLibA PROPERTY SYCL_SEPARABLE_COMPILATION)
if(NOT separable)
  message(FATAL_ERROR "SYCL_SEPARABLE_COMPILATION was not initialized")
endif()
unset(CMAKE_SYCL_SEPARABLE_COMPILATION)

add_library(SYCLSeparateLibB SHARED launch.sycl)
target_compile_features(SYCLSeparateLibB PRIVATE sycl_cxx_std_17)
set_target_properties(SYCLSeparateLibB PROPERTIES
  SYCL_SEPARABLE_COMPILATION ON WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(SYCLSeparateLibB PRIVATE SYCLSeparateLibA)
add_library(SYCLSeparateLibC SHARED launch.sycl)
target_compile_features(SYCLSeparateLibC PRIVATE sycl_cxx_std_17)
set_target_properties(SYCLSeparateLibB SYCLSeparateLibC PROPERTIES
  SYCL_VISIBILITY_PRESET hidden
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/main")
set_target_properties(SYCLSeparateLibC PROPERTIES
  SYCL_SEPARABLE_COMPILATION ON WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(SYCLSeparateLibC PRIVATE
  KERNEL_NAME=SeparateCompilationKernel2 LAUNCH_FUNCTION=launch_kernel2)
target_link_libraries(SYCLSeparateLibC PRIVATE SYCLSeparateLibA)
add_subdirectory(main)
