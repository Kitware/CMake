# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__LINUX_COMPILER_NVIDIA)
  return()
endif()
set(__LINUX_COMPILER_NVIDIA 1)

include(Platform/Linux-PGI)

macro(__linux_compiler_nvhpc lang)
  __linux_compiler_pgi(${lang})
endmacro()
