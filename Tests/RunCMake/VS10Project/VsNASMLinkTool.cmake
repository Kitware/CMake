enable_language(C)
include(CheckLanguage)
check_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER)
  enable_language(ASM_NASM)
  # A target whose link language is ASM_NASM must not force CMAKE_LINKER into
  # <LinkToolExe>; the Visual Studio toolset performs the link.
  add_executable(nasmexe nasm_main.asm)
endif()
