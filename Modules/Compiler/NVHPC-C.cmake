include(Compiler/PGI-C)
include(Compiler/NVHPC)

# Needed so that we support `LANGUAGE` property correctly
set(CMAKE_C_COMPILE_OPTIONS_EXPLICIT_LANGUAGE -x c)

__compiler_nvhpc(C)
