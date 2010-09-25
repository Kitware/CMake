# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that the selected ASM_NASM "compiler" works.
# For assembler this can only check whether the compiler has been found,
# because otherwise there would have to be a separate assembler source file
# for each assembler on every architecture.

SET(ASM_DIALECT "_NASM")
INCLUDE(CMakeTestASMCompiler)
SET(ASM_DIALECT)
