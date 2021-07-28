include(RunCMake)

run_cmake(NotEnabledLanguage)
run_cmake(NonExistentLanguage)
run_cmake(UnknownArgument)

run_cmake(CheckCSourceCompiles)
run_cmake(CheckCXXSourceCompiles)

if (APPLE)
  run_cmake(CheckOBJCSourceCompiles)
  run_cmake(CheckOBJCXXSourceCompiles)
endif()

if (CMAKE_Fortran_COMPILER_ID)
  run_cmake(CheckFortranSourceCompiles)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckCUDASourceCompiles)
endif()

if(CMake_TEST_ISPC)
  run_cmake(CheckISPCSourceCompiles)
endif()

if(CMake_TEST_HIP)
  run_cmake(CheckHIPSourceCompiles)
endif()
