include(RunCMake)

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU|LCC")
  run_cmake(CheckCLinkerFlag)
  run_cmake(CheckCXXLinkerFlag)
  if (APPLE)
    run_cmake(CheckOBJCLinkerFlag)
    run_cmake(CheckOBJCXXLinkerFlag)
  endif()
endif()

if (CMAKE_Fortran_COMPILER_ID MATCHES "GNU|LCC")
  run_cmake(CheckFortranLinkerFlag)
endif()

if (CMake_TEST_CUDA)
  run_cmake(CheckCUDALinkerFlag)
endif()

if (CMake_TEST_HIP)
  run_cmake(CheckHIPLinkerFlag)
endif()
