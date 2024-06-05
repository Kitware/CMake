# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# support for the MS assembler, masm and masm64

# Load the generic ASMInformation file:
set(ASM_DIALECT "_MASM")
include(Internal/CMakeASMLinkerInformation)
set(ASM_DIALECT)
