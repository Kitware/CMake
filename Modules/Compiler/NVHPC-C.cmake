include(Compiler/PGI-C)
include(Compiler/NVHPC)

# Needed so that we support `LANGUAGE` property correctly
set(CMAKE_C_COMPILE_OPTIONS_EXPLICIT_LANGUAGE -x c)

if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 20.11)
  set(CMAKE_C17_STANDARD_COMPILE_OPTION  -std=c17)
  set(CMAKE_C17_EXTENSION_COMPILE_OPTION -std=gnu17)
endif()

__compiler_nvhpc(C)
