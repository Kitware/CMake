include(RunCMake)

run_cmake(NotEnabledLanguage)
run_cmake(NonExistentLanguage)

run_cmake(CheckCCompilerFlag)
run_cmake(CheckCXXCompilerFlag)

if (APPLE)
  run_cmake(CheckOBJCCompilerFlag)
  run_cmake(CheckOBJCXXCompilerFlag)
endif()

if (CMAKE_Fortran_COMPILER_ID)
  run_cmake(CheckFortranCompilerFlag)
endif()
