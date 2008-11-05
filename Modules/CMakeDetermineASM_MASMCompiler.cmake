# Find the MS assembler (masm or masm64)

SET(ASM_DIALECT "_MASM")

# if we are using the 64bit cl compiler, assume we also want the 64bit assembler
IF(CMAKE_CL_64)
   SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ml64)
ELSE(CMAKE_CL_64)
   SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ml)
ENDIF(CMAKE_CL_64)

INCLUDE(CMakeDetermineASMCompiler)
SET(ASM_DIALECT)
