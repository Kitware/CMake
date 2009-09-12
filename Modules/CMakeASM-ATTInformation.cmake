# support for AT&T syntax assemblers, e.g. GNU as

SET(ASM_DIALECT "-ATT")
# *.S files are supposed to be preprocessed, so they should not be passed to
# assembler but should be processed by gcc
SET(CMAKE_ASM${ASM_DIALECT}_SOURCE_FILE_EXTENSIONS s;asm)
INCLUDE(CMakeASMInformation)
SET(ASM_DIALECT)
