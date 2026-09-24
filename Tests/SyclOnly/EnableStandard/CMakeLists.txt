cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(EnableStandard SYCL)

add_library(SYCLStatic20 STATIC static.sycl)
add_library(SYCLDynamic20 SHARED shared.sycl)
set_property(TARGET SYCLDynamic20 PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(SYCLStatic20 PROPERTIES
  SYCL_CXX_STANDARD 20 SYCL_CXX_STANDARD_REQUIRED TRUE)
target_compile_features(SYCLDynamic20 PRIVATE sycl_cxx_std_20)
add_executable(SyclOnlyEnableStandard main.sycl)
target_link_libraries(SyclOnlyEnableStandard PRIVATE SYCLStatic20 SYCLDynamic20)
