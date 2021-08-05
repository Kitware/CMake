include(Compiler/PGI-CXX)
include(Compiler/NVHPC)

# Needed so that we support `LANGUAGE` property correctly
set(CMAKE_CXX_COMPILE_OPTIONS_EXPLICIT_LANGUAGE -x c++)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 20.11)
  set(CMAKE_CXX20_STANDARD_COMPILE_OPTION  -std=c++20)
  set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION -std=gnu++20)
endif()

__compiler_nvhpc(CXX)
