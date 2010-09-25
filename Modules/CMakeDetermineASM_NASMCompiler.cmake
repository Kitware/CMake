# Find the nasm assembler

SET(CMAKE_ASM_NASM_COMPILER_INIT nasm)

IF(NOT CMAKE_ASM_NASM_COMPILER)
  FIND_PROGRAM(CMAKE_ASM_NASM_COMPILER nasm
    "$ENV{ProgramFiles}/NASM")
ENDIF(NOT CMAKE_ASM_NASM_COMPILER)

# Load the generic DetermineASM compiler file with the DIALECT set properly:
SET(ASM_DIALECT "_NASM")
INCLUDE(CMakeDetermineASMCompiler)
SET(ASM_DIALECT)
