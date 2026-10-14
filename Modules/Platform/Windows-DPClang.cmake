# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard()

macro(__windows_compiler_dpclang lang)
  set(_dpclang_${lang}_version "${CMAKE_${lang}_COMPILER_VERSION}")
  set(CMAKE_${lang}_COMPILER_VERSION "${CMAKE_${lang}_COMPILER_VERSION_INTERNAL}")
  include(Platform/Windows-Clang-${lang})
  set(CMAKE_${lang}_COMPILER_VERSION "${_dpclang_${lang}_version}")
  unset(_dpclang_${lang}_version)
endmacro()
