include(Compiler/PGI-Fortran)
include(Compiler/NVHPC)
__compiler_nvhpc(Fortran)
set(CMAKE_Fortran_PREPROCESS_SOURCE_EXCLUDE_FLAGS_REGEX "(^| )-Werror +[a-z][a-z-]+( |$)")
