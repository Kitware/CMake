
# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that the selected ASM_MASM "compiler" (should be masm or masm64) 
# can actually "compile" and link the most basic of programs.   If not, a 
# fatal error is set and cmake stops processing commands and will not generate
# any makefiles or projects.

SET(ASM_DIALECT "_MASM")
INCLUDE(CMakeTestASMCompiler)
SET(ASM_DIALECT)
