# Assembler information is detected once and written to
# CMakeASM<dialect>Compiler.cmake, then reloaded from that file on every later
# configure.  A field that detection computes but the file does not record
# comes back empty, so a re-configure sees a different assembler than the one
# that was detected.  The -check script reloads the file and verifies this.

enable_language(ASM)

# Detection must identify the assembler in the first place.
if(NOT CMAKE_ASM_COMPILER_ID)
  message(SEND_ERROR "CMAKE_ASM_COMPILER_ID was not detected")
endif()
if(NOT CMAKE_ASM_COMPILER_FRONTEND_VARIANT)
  message(SEND_ERROR "CMAKE_ASM_COMPILER_FRONTEND_VARIANT was not detected")
endif()
