include(Compiler/PGI-CXX)
include(Compiler/NVHPC)

# Needed so that we support `LANGUAGE` property correctly
set(CMAKE_CXX_COMPILE_OPTIONS_EXPLICIT_LANGUAGE -x c++)

# Required since as of NVHPC 21.03 the `-MD` flag implicitly
# implies `-E` and therefore compilation and dependency generation
# can't occur in the same invocation
set(CMAKE_CXX_DEPENDS_EXTRA_COMMANDS "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -x c++ -M <SOURCE> -MT <OBJECT> -MD<DEP_FILE>")

__compiler_nvhpc(CXX)
