include(RunCMake)

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU|LCC")
  run_cmake(CheckLinkerFlagC)
  run_cmake(CheckLinkerFlagCXX)
  if (APPLE)
    run_cmake(CheckLinkerFlagOBJC)
    run_cmake(CheckLinkerFlagOBJCXX)
  endif()
endif()

if (CMAKE_Fortran_COMPILER_ID MATCHES "GNU|LCC")
  run_cmake(CheckLinkerFlagFortran)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckLinkerFlagCUDA)
endif()

if (CMake_TEST_HIP)
  run_cmake(CheckLinkerFlagHIP)
endif()
