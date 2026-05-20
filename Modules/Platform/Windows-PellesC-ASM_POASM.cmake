# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include (Platform/Windows-PellesC)
__windows_compiler_pellesc(ASM_POASM)

if(_PellesC_ARCH STREQUAL "x64")
  set(_PellesC_ASM_POASM_FLAGS " -Aamd64")
elseif(_PellesC_ARCH STREQUAL "x86")
  set(_PellesC_ASM_POASM_FLAGS " -Aia32")
else()
  set(_PellesC_ASM_POASM_FLAGS "")
endif()

set(CMAKE_ASM_POASM_COMPILE_OBJECT
  "<CMAKE_ASM_POASM_COMPILER> <DEFINES> <INCLUDES>${_PellesC_ASM_POASM_FLAGS} <FLAGS> -Fo<OBJECT> <SOURCE>")

string(APPEND CMAKE_ASM_POASM_FLAGS_DEBUG_INIT " -Zi")
string(APPEND CMAKE_ASM_POASM_FLAGS_RELEASE_INIT "")
string(APPEND CMAKE_ASM_POASM_FLAGS_RELWITHDEBINFO_INIT " -Zi")
string(APPEND CMAKE_ASM_POASM_FLAGS_MINSIZEREL_INIT "")

unset(_PellesC_ASM_POASM_FLAGS)
