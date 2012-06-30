include(Compiler/GNU)
__compiler_gnu(Fortran)

set(CMAKE_Fortran_FORMAT_FIXED_FLAG "-ffixed-form")
set(CMAKE_Fortran_FORMAT_FREE_FLAG "-ffree-form")

# No -DNDEBUG for Fortran.
set(CMAKE_Fortran_FLAGS_MINSIZEREL_INIT "-Os")
set(CMAKE_Fortran_FLAGS_RELEASE_INIT "-O3")

# We require updates to CMake C++ code to support preprocessing rules
# for Fortran.
set(CMAKE_Fortran_CREATE_PREPROCESSED_SOURCE)
set(CMAKE_Fortran_CREATE_ASSEMBLY_SOURCE)

# Fortran-specific feature flags.
set(CMAKE_Fortran_MODDIR_FLAG -J)
