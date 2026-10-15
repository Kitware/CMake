# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Standalone device linking for the IntelLLVM and DPClang DPC++ drivers.
set(CMAKE_SYCL_COMPILER_HAS_DEVICE_LINK_PHASE TRUE)
set(CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_ON "-fgpu-rdc")
set(CMAKE_SYCL_COMPILE_OPTIONS_SEPARABLE_COMPILATION_OFF "-fno-gpu-rdc")
set(CMAKE_SYCL_DEVICE_LINK_OPTIONS "-fsycl;-fsycl-link")
if(NOT CMAKE_SYCL_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS_WHOLE_ARCHIVE "-Wl,--whole-archive")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS_NO_WHOLE_ARCHIVE "-Wl,--no-whole-archive")
elseif(CMAKE_SYCL_COMPILER_ID STREQUAL "IntelLLVM")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS_WHOLE_ARCHIVE
    "-Qoption,link,--whole-archive")
  set(CMAKE_SYCL_DEVICE_LINK_OPTIONS_NO_WHOLE_ARCHIVE
    "-Qoption,link,--no-whole-archive")
endif()
