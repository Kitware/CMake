# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Compiler options depend on the underlying compiler, not the AdaptiveCpp
# release number.  Restore the public version after initializing the options.
set(_CMAKE_SYCL_ADAPTIVECPP_VERSION "${CMAKE_SYCL_COMPILER_VERSION}")
set(CMAKE_SYCL_COMPILER_VERSION "${CMAKE_SYCL_HOST_COMPILER_VERSION}")
set(_CMAKE_SYCL_REQUIRED_FLAG "")
if(CMAKE_SYCL_HOST_COMPILER_ID STREQUAL "Clang")
  include(Compiler/Clang-SYCL)
elseif(CMAKE_SYCL_HOST_COMPILER_ID STREQUAL "GNU")
  include(Compiler/GNU)
  __compiler_gnu(SYCL)
  __compiler_gnu_cxx_standards(SYCL)
  set(CMAKE_SYCL_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN
    "-fvisibility-inlines-hidden")
else()
  message(FATAL_ERROR
    "AdaptiveCpp host compiler '${CMAKE_SYCL_HOST_COMPILER_ID}' is not supported.")
endif()
set(CMAKE_SYCL_COMPILER_VERSION "${_CMAKE_SYCL_ADAPTIVECPP_VERSION}")
unset(_CMAKE_SYCL_ADAPTIVECPP_VERSION)

foreach(std IN ITEMS 98 11 14)
  unset(CMAKE_SYCL${std}_STANDARD_COMPILE_OPTION)
  unset(CMAKE_SYCL${std}_EXTENSION_COMPILE_OPTION)
endforeach()
