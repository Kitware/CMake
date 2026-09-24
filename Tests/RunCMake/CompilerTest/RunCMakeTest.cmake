include(RunCMake)

if(CMake_TEST_SYCL)
  list(APPEND RunCMake_TEST_OPTIONS -Wno-experimental)
endif()

run_cmake(C)
run_cmake(CXX)

if(CMake_TEST_CUDA)
  run_cmake(CUDA)
endif()

if(CMake_TEST_Fortran)
  run_cmake(Fortran)
endif()

if(CMake_TEST_HIP)
  run_cmake(HIP)
endif()

if(CMake_TEST_SYCL)
  run_cmake(SYCL)
endif()

if(CMake_TEST_ISPC)
  run_cmake(ISPC)
endif()

if(CMake_TEST_OBJC)
  run_cmake(OBJC)
  run_cmake(OBJCXX)
endif()
