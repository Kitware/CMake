include(RunCMake)

run_cmake(C)
run_cmake(CXX)

if(APPLE)
  run_cmake(OBJC)
  run_cmake(OBJCXX)
endif()

if(CMake_TEST_CUDA)
  run_cmake(CUDA)
endif()

run_cmake(TestBigEndian-NoLang)
