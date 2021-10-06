# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__COMPILER_NVHPC)
  return()
endif()
set(__COMPILER_NVHPC 1)

include(Compiler/PGI)

macro(__compiler_nvhpc lang)
  # Logic specific to NVHPC.
endmacro()
