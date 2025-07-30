if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/GeneratorTest-1.2.3-Linux.AppImage")
  set(RunCMake_TEST_FAILED "AppImage package not generated")
endif()
