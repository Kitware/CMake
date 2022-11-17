cmake_minimum_required(VERSION 3.8)
project(CheckIPOSupported-CUDA LANGUAGES CUDA)

cmake_policy(SET CMP0069 NEW)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(NOT ipo_supported AND CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA"
   AND CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 11.2)
  message(FATAL_ERROR "CheckIPOSupported failed to correctly identify NVIDIA CUDA IPO support")
endif()

set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)

add_library(foo STATIC foo.cu)
set_target_properties(foo PROPERTIES
      WINDOWS_EXPORT_ALL_SYMBOLS ON
      POSITION_INDEPENDENT_CODE ON)

add_library(bar SHARED bar.cu)
set_target_properties(bar PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(bar PRIVATE foo)

add_executable(CheckIPOSupported-CUDA main.cu)
target_link_libraries(CheckIPOSupported-CUDA PUBLIC bar)

enable_testing()
add_test(NAME CheckIPOSupported-CUDA COMMAND CheckIPOSupported-CUDA)
