# determine the compiler to use for ASM using AT&T syntax, e.g. GNU as

SET(ASM_DIALECT "-ATT")
SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ${_CMAKE_TOOLCHAIN_PREFIX}gas ${_CMAKE_TOOLCHAIN_PREFIX}as)
INCLUDE(CMakeDetermineASMCompiler)
SET(ASM_DIALECT)
