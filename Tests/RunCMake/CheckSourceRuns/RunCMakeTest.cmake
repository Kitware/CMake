include(RunCMake)

run_cmake(NotEnabledLanguage)
run_cmake(NonExistentLanguage)
run_cmake(UnknownArgument)

run_cmake(CheckCSourceRuns)
run_cmake(CheckCXXSourceRuns)

if (APPLE)
  run_cmake(CheckOBJCSourceRuns)
  run_cmake(CheckOBJCXXSourceRuns)
endif()

if (CMAKE_Fortran_COMPILER_ID)
  run_cmake(CheckFortranSourceRuns)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckCUDASourceRuns)
endif()

if (CMake_TEST_HIP)
  run_cmake(CheckHIPSourceRuns)
endif()
