include(CheckLanguage)
check_language(Fortran)
if(NOT CMAKE_Fortran_COMPILER)
  # No Fortran compiler, skipping Fortran test
  return()
endif()

if(SET_CMP0129)
  cmake_policy(SET CMP0129 ${SET_CMP0129})
endif()

enable_language(Fortran)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
include(CompareCompilerVersion.cmake)
compare_compiler_version(Fortran)
