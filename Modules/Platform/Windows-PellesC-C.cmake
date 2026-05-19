# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include (Platform/Windows-PellesC)
__windows_compiler_pellesc(C)

if(_PellesC_ARCH)
  set(_PellesC_C_OBJ_FLAGS " -T${_PellesC_ARCH}-coff")
else()
  set(_PellesC_C_OBJ_FLAGS "")
endif()
if(_PellesC_ARCH STREQUAL "x64")
  set(_PellesC_C_ASM_FLAGS " -Tx64-asm")
elseif(_PellesC_ARCH STREQUAL "x86")
  set(_PellesC_C_ASM_FLAGS " -Tx86-asm")
else()
  set(_PellesC_C_ASM_FLAGS "")
endif()

set(CMAKE_C_COMPILE_OBJECT
  "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES>${_PellesC_C_OBJ_FLAGS} <FLAGS> -Fo<OBJECT> -c <SOURCE>")

set(CMAKE_C_CREATE_PREPROCESSED_SOURCE
  "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES>${_PellesC_C_OBJ_FLAGS} <FLAGS> -E <SOURCE> ><PREPROCESSED_SOURCE>")

set(CMAKE_C_CREATE_ASSEMBLY_SOURCE
  "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES>${_PellesC_C_ASM_FLAGS} <FLAGS> -Fo<ASSEMBLY_SOURCE> <SOURCE>")

# Enable Pelles C's Microsoft extensions to use Windows APIs.
# The flag also disables standard definitions, so add them.
string(APPEND CMAKE_C_FLAGS_INIT " -Ze -D__STDC__=1 -D__STDC_VERSION__=__POCC_STDC_VERSION__")

string(APPEND CMAKE_C_FLAGS_DEBUG_INIT " -Zi -Ob0")
string(APPEND CMAKE_C_FLAGS_RELEASE_INIT " -Ot -Ob2 -DNDEBUG=1")
string(APPEND CMAKE_C_FLAGS_RELWITHDEBINFO_INIT " -Zi -Ot -Ob1 -DNDEBUG=1")
string(APPEND CMAKE_C_FLAGS_MINSIZEREL_INIT " -Os -Ob1 -DNDEBUG=1")

set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib")

unset(_PellesC_C_OBJ_FLAGS)
unset(_PellesC_C_ASM_FLAGS)
