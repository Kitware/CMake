include(RunCMake)

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  run_cmake(CheckCLinkerFlag)
  run_cmake(CheckCXXLinkerFlag)
  if (APPLE)
    run_cmake(CheckOBJCLinkerFlag)
    run_cmake(CheckOBJCXXLinkerFlag)
  endif()
endif()

if (CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
  run_cmake(CheckFortranLinkerFlag)
endif()
