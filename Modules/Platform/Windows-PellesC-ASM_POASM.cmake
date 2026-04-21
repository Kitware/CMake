# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include (Platform/Windows-PellesC)
__windows_compiler_pellesc(ASM_POASM)

set(CMAKE_ASM_POASM_COMPILE_OBJECT
  "<CMAKE_ASM_POASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -Fo<OBJECT> <SOURCE>")
