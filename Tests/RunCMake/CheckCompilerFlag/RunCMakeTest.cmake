include(RunCMake)

run_cmake(NotEnabledLanguage)
run_cmake(NonExistentLanguage)

run_cmake(CheckCCompilerFlag)
run_cmake(CheckCXXCompilerFlag)
run_cmake(CheckCompilerFlagC)
run_cmake(CheckCompilerFlagCXX)

if (APPLE)
  run_cmake(CheckCompilerFlagOBJC)
  run_cmake(CheckCompilerFlagOBJCXX)
endif()

if (CMAKE_Fortran_COMPILER_ID)
  run_cmake(CheckCompilerFlagFortran)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckCompilerFlagCUDA)
endif()

if(CMake_TEST_ISPC)
  run_cmake(CheckCompilerFlagISPC)
endif()

if(CMake_TEST_HIP)
  run_cmake(CheckCompilerFlagHIP)
endif()

if(APPLE)
  run_cmake(HeaderpadWorkaround)
endif()

if(CMake_TEST_Swift)
  run_cmake(CheckCompilerFlagSwift)
endif()
