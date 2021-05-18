include(Compiler/PGI-CXX)
include(Compiler/NVHPC)

# Needed so that we support `LANGUAGE` property correctly
set(CMAKE_CXX_COMPILE_OPTIONS_EXPLICIT_LANGUAGE -x c++)

__compiler_nvhpc(CXX)
