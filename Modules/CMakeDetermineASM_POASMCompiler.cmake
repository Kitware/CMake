# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

set(ASM_DIALECT "_POASM")
set(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT poasm)
include(CMakeDetermineASMCompiler)
unset(ASM_DIALECT)
