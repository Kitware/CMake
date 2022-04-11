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

if (CMake_TEST_CUDA)
  run_cmake(CheckCUDACompilerFlag)
endif()

if(CMake_TEST_ISPC)
  run_cmake(CheckISPCCompilerFlag)
endif()

if(CMake_TEST_HIP)
  run_cmake(CheckHIPCompilerFlag)
endif()

if(APPLE)
  run_cmake_with_options(HeaderpadWorkaround --debug-trycompile)
endif()
