# support for AT&T syntax assemblers, e.g. GNU as

SET(ASM_DIALECT "-ATT")
SET(CMAKE_ASM${ASM_DIALECT}_SOURCE_FILE_EXTENSIONS s;S;asm)
INCLUDE(CMakeASMInformation)
SET(ASM_DIALECT)
