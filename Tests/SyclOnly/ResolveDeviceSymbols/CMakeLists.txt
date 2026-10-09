cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(ResolveDeviceSymbols CXX SYCL)
include(../utils/Test.cmake)
# Remove gate when IntelLLVM -fsycl-link bug is fixed
if(CMAKE_SYCL_COMPILER_ID STREQUAL "IntelLLVM")
  sycl_not_supported("IntelLLVM -fsycl-link cannot produce a single device-link output")
  return()
endif()

if(CMAKE_GENERATOR MATCHES "Xcode" OR
   NOT CMAKE_SYCL_DEVICE_LINK_OPTIONS)
  sycl_not_supported("This compiler/generator does not support standalone SYCL device linking")
  return()
endif()

add_library(SYCLResolveDeviceDepsA STATIC ../SeparateCompilation/device_extra.sycl)
add_library(SYCLResolveDeviceDepsB STATIC ../SeparateCompilation/device.sycl)
target_compile_features(SYCLResolveDeviceDepsA PUBLIC sycl_cxx_std_17)
target_compile_features(SYCLResolveDeviceDepsB PUBLIC sycl_cxx_std_17)
set_target_properties(SYCLResolveDeviceDepsA SYCLResolveDeviceDepsB PROPERTIES
  SYCL_SEPARABLE_COMPILATION ON POSITION_INDEPENDENT_CODE ON)

add_library(SYCLResolveDeviceLib STATIC ../SeparateCompilation/launch.sycl available.sycl)
set_target_properties(SYCLResolveDeviceLib PROPERTIES
  SYCL_SEPARABLE_COMPILATION ON SYCL_RESOLVE_DEVICE_SYMBOLS ON
  POSITION_INDEPENDENT_CODE ON)
target_link_libraries(SYCLResolveDeviceLib PRIVATE SYCLResolveDeviceDepsB SYCLResolveDeviceDepsA)
if(CMAKE_AR)
  add_custom_command(TARGET SYCLResolveDeviceLib POST_BUILD
    COMMAND "${CMAKE_COMMAND}" "-DARCHIVER=${CMAKE_AR}"
      "-DLIBRARY=$<TARGET_FILE:SYCLResolveDeviceLib>"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/verify.cmake"
    VERBATIM)
endif()

add_executable(SyclOnlyResolveDeviceSymbols main.cxx)
set_target_properties(SyclOnlyResolveDeviceSymbols PROPERTIES
  SYCL_SEPARABLE_COMPILATION OFF SYCL_RESOLVE_DEVICE_SYMBOLS OFF)
target_link_libraries(SyclOnlyResolveDeviceSymbols PRIVATE SYCLResolveDeviceLib)
sycl_add_runtime_test(ResolveDeviceSymbols SyclOnlyResolveDeviceSymbols)

add_executable(SyclOnlyResolveDeviceSymbolsMixed main.cxx mixed.sycl)
set_target_properties(SyclOnlyResolveDeviceSymbolsMixed PROPERTIES
  LINKER_LANGUAGE SYCL SYCL_SEPARABLE_COMPILATION ON)
add_library(SYCLMixedDeviceDep STATIC mixed_device.sycl)
set_property(TARGET SYCLMixedDeviceDep PROPERTY SYCL_SEPARABLE_COMPILATION ON)
target_compile_definitions(SyclOnlyResolveDeviceSymbolsMixed PRIVATE TEST_MIXED_KERNEL)
target_link_libraries(SyclOnlyResolveDeviceSymbolsMixed PRIVATE SYCLResolveDeviceLib SYCLMixedDeviceDep)
sycl_add_runtime_test(ResolveDeviceSymbolsMixed SyclOnlyResolveDeviceSymbolsMixed)
