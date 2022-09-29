include(RunCMake)

run_cmake(C)
run_cmake(CXX)

if(CMake_TEST_OBJC)
  run_cmake(OBJC)
  run_cmake(OBJCXX)
endif()
