include(RunCMake)

run_cmake(NotEnabledLanguage)
run_cmake(NonExistentLanguage)
run_cmake(UnknownArgument)

run_cmake(CheckCSourceCompiles)
run_cmake(CheckCXXSourceCompiles)
run_cmake(CheckSourceCompilesC)
run_cmake(CheckSourceCompilesCXX)

if (APPLE)
  run_cmake(CheckOBJCSourceCompiles)
  run_cmake(CheckOBJCXXSourceCompiles)
  run_cmake(CheckSourceCompilesOBJC)
  run_cmake(CheckSourceCompilesOBJCXX)
endif()

if (CMAKE_Fortran_COMPILER_ID)
  run_cmake(CheckSourceCompilesFortran)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckSourceCompilesCUDA)
endif()

if(CMake_TEST_ISPC)
  run_cmake(CheckSourceCompilesISPC)
endif()

if(CMake_TEST_HIP)
  run_cmake(CheckSourceCompilesHIP)
endif()

if(CMake_TEST_Swift)
  run_cmake(CheckSourceCompilesSwift)
endif()
