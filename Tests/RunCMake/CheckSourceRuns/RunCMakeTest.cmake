include(RunCMake)

run_cmake(NotEnabledLanguage)
run_cmake(NonExistentLanguage)
run_cmake(UnknownArgument)

run_cmake(CheckCSourceRuns)
run_cmake(CheckCXXSourceRuns)
run_cmake(CheckSourceRunsC)
run_cmake(CheckSourceRunsCXX)

if (APPLE)
  run_cmake(CheckOBJCSourceRuns)
  run_cmake(CheckOBJCXXSourceRuns)
  run_cmake(CheckSourceRunsOBJC)
  run_cmake(CheckSourceRunsOBJCXX)
endif()

if (CMAKE_Fortran_COMPILER_ID)
  run_cmake(CheckSourceRunsFortran)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckSourceRunsCUDA)
endif()

if (CMake_TEST_HIP)
  run_cmake(CheckSourceRunsHIP)
endif()
