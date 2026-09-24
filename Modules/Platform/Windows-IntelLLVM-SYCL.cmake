# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

if(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  include(Platform/Windows-Clang-SYCL)
  return()
endif()

include(Platform/Windows-IntelLLVM)
include(Platform/Windows-Clang)
set(_COMPILE_SYCL " ${_CMAKE_SYCL_REQUIRED_FLAG}")
__windows_compiler_intel(SYCL)

foreach(rule CREATE_SHARED_LIBRARY CREATE_SHARED_MODULE LINK_EXECUTABLE)
  string(REPLACE "<CMAKE_SYCL_COMPILER>" "<CMAKE_SYCL_COMPILER> ${_CMAKE_SYCL_REQUIRED_FLAG}"
    CMAKE_SYCL_${rule} "${CMAKE_SYCL_${rule}}")
endforeach()
unset(rule)
if(CMAKE_GENERATOR MATCHES "Visual Studio")
  # Use the same clang-cl driver/linker boundary as Clang's offload rules.
  __windows_compiler_clang_msvc_offload_link(SYCL "${_CMAKE_SYCL_REQUIRED_FLAG}" "")
endif()
unset(_COMPILE_SYCL)
