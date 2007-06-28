
# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected ASM compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
IF(CMAKE_ASM${ASM_DIALECT}_COMPILER)
  SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_WORKS 1 CACHE INTERNAL "")
ELSE(CMAKE_ASM${ASM_DIALECT}_COMPILER)
  SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_WORKS 0 CACHE INTERNAL "")
ENDIF(CMAKE_ASM${ASM_DIALECT}_COMPILER)
