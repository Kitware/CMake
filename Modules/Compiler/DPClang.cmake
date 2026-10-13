# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard()

# DPClang's public version describes DPC++, not the underlying Clang. Use the
# latter for Clang's feature thresholds while preserving the public version.
macro(__compiler_dpclang lang)
  set(_dpclang_${lang}_version "${CMAKE_${lang}_COMPILER_VERSION}")
  set(CMAKE_${lang}_COMPILER_VERSION "${CMAKE_${lang}_COMPILER_VERSION_INTERNAL}")
  include(Compiler/Clang-${lang})
  set(CMAKE_${lang}_COMPILER_VERSION "${_dpclang_${lang}_version}")
  unset(_dpclang_${lang}_version)
endmacro()
