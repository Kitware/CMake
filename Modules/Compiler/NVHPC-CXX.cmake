include(Compiler/PGI-CXX)
include(Compiler/NVHPC)

# Needed so that we support `LANGUAGE` property correctly
set(CMAKE_CXX_COMPILE_OPTIONS_EXPLICIT_LANGUAGE -x c++)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 20.11)
  set(CMAKE_CXX20_STANDARD_COMPILE_OPTION  -std=c++20)
  set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION -std=gnu++20)
endif()

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 21.07)
  set(CMAKE_DEPFILE_FLAGS_CXX "-MD -MT <DEP_TARGET> -MF <DEP_FILE>")
  set(CMAKE_CXX_DEPFILE_FORMAT gcc)
  set(CMAKE_CXX_DEPENDS_USE_COMPILER TRUE)
else()
  # Before NVHPC 21.07 the `-MD` flag implicitly
  # implies `-E` and therefore compilation and dependency generation
  # can't occur in the same invocation
  set(CMAKE_CXX_DEPENDS_EXTRA_COMMANDS "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -x c++ -M <SOURCE> -MT <OBJECT> -MD<DEP_FILE>")
endif()
__compiler_nvhpc(CXX)
