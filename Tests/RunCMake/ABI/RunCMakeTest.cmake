include(RunCMake)

if(CMake_TEST_SYCL)
  list(APPEND RunCMake_TEST_OPTIONS -Wno-experimental)
endif()

run_cmake(C)
run_cmake(CXX)

if(APPLE)
  run_cmake(OBJC)
  run_cmake(OBJCXX)
endif()

if(CMake_TEST_CUDA)
  run_cmake(CUDA)
endif()

if(CMake_TEST_SYCL)
  run_cmake(SYCL)
endif()

run_cmake(TestBigEndian-NoLang)
