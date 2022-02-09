# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_INTEL)
  return()
endif()
set(__WINDOWS_INTEL 1)

include(Platform/Windows-MSVC)
macro(__windows_compiler_intel lang)
  __windows_compiler_msvc(${lang})

  set(CMAKE_DEPFILE_FLAGS_${lang} "-QMD -QMT <DEP_TARGET> -QMF <DEP_FILE>")
  set(CMAKE_${lang}_DEPFILE_FORMAT gcc)
endmacro()
