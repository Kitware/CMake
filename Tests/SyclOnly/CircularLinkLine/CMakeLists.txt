cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(CircularLinkLine SYCL CXX)
include(../utils/Test.cmake)
# Remove gate when IntelLLVM -fsycl-link bug is fixed
if(CMAKE_SYCL_COMPILER_ID STREQUAL "IntelLLVM")
  sycl_not_supported("IntelLLVM -fsycl-link cannot produce a single device-link output")
  return()
endif()
if(NOT CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_ON OR
   NOT CMAKE_SYCL_DEVICE_LINK_OPTIONS OR
   CMAKE_GENERATOR MATCHES "Xcode")
  sycl_not_supported("This compiler/generator has no standalone SYCL device link for circular archives")
  return()
endif()
set(CMAKE_SYCL_CXX_STANDARD 17)
add_library(SYCLCircularDeviceLinking1 STATIC file1.sycl)
add_library(SYCLCircularDeviceLinking2 STATIC file2.sycl)
add_library(SYCLCircularDeviceLinking3 STATIC file3.sycl)
add_executable(SyclOnlyCircularLinkLine main.sycl)
# CUDA's generated device-link step de-duplicates its archive inputs.  Request
# SYCL's corresponding separate step rather than testing the native driver's
# handling of a repeated host archive line (which is compiler-specific).
set_target_properties(SyclOnlyCircularLinkLine PROPERTIES
  SYCL_SEPARABLE_COMPILATION ON SYCL_RESOLVE_DEVICE_SYMBOLS ON
  POSITION_INDEPENDENT_CODE ON LINKER_LANGUAGE CXX)
target_link_libraries(SYCLCircularDeviceLinking1 PUBLIC SYCLCircularDeviceLinking2)
target_link_libraries(SYCLCircularDeviceLinking2 PUBLIC SYCLCircularDeviceLinking3)
target_link_libraries(SYCLCircularDeviceLinking3 PUBLIC SYCLCircularDeviceLinking1)
target_link_libraries(SyclOnlyCircularLinkLine PRIVATE SYCLCircularDeviceLinking3)
set_target_properties(SYCLCircularDeviceLinking1 SYCLCircularDeviceLinking2
  SYCLCircularDeviceLinking3 PROPERTIES SYCL_SEPARABLE_COMPILATION ON)
sycl_add_runtime_test(CircularLinkLine SyclOnlyCircularLinkLine)
