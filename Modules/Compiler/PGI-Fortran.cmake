include(Compiler/PGI)
__compiler_pgi(Fortran)

set(CMAKE_Fortran_FORMAT_FIXED_FLAG "-Mnofreeform")
set(CMAKE_Fortran_FORMAT_FREE_FLAG "-Mfreeform")

SET(CMAKE_Fortran_FLAGS_INIT "${CMAKE_Fortran_FLAGS_INIT} -Mpreprocess -Kieee")
SET(CMAKE_Fortran_FLAGS_DEBUG_INIT "${CMAKE_Fortran_FLAGS_DEBUG_INIT} -Mbounds")

# We require updates to CMake C++ code to support preprocessing rules
# for Fortran.
SET(CMAKE_Fortran_CREATE_PREPROCESSED_SOURCE)
SET(CMAKE_Fortran_CREATE_ASSEMBLY_SOURCE)

SET(CMAKE_Fortran_MODDIR_FLAG "-module ")
