# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include(Platform/Windows-Clang)
set(_COMPILE_SYCL_MSVC " ${_CMAKE_SYCL_REQUIRED_FLAG}")
__windows_compiler_clang(SYCL)
if(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT STREQUAL "GNU" AND
   CMAKE_SYCL_SIMULATE_ID STREQUAL "MSVC")
  unset(CMAKE_INCLUDE_SYSTEM_FLAG_SYCL_WARNING)
  if(CMAKE_SYCL_COMPILER_ID MATCHES "^(Clang|IntelLLVM)$")
    # Let the driver apply CRT defines to the host compilation only and select
    # the matching libsycl dependency, including the debug runtime.
    set(CMAKE_SYCL_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreaded -fms-runtime-lib=static)
    set(CMAKE_SYCL_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDLL -fms-runtime-lib=dll)
    set(CMAKE_SYCL_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebug -fms-runtime-lib=static_dbg)
    set(CMAKE_SYCL_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebugDLL -fms-runtime-lib=dll_dbg)
  endif()
endif()

if(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  foreach(rule CREATE_SHARED_LIBRARY CREATE_SHARED_MODULE LINK_EXECUTABLE)
    string(REPLACE "<CMAKE_SYCL_COMPILER>" "<CMAKE_SYCL_COMPILER> ${_CMAKE_SYCL_REQUIRED_FLAG}"
      CMAKE_SYCL_${rule} "${CMAKE_SYCL_${rule}}")
    # Let the SYCL driver add its runtime libraries.
    string(REPLACE "-nostartfiles -nostdlib " ""
      CMAKE_SYCL_${rule} "${CMAKE_SYCL_${rule}}")
  endforeach()
  unset(rule)
  set(CMAKE_SYCL_VERBOSE_FLAG "-v")
else()
  __windows_compiler_clang_msvc_offload_link(SYCL "${_CMAKE_SYCL_REQUIRED_FLAG}" "")
  set(CMAKE_SYCL_VERBOSE_FLAG "-clang:-v")
endif()
unset(_COMPILE_SYCL_MSVC)
