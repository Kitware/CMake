if(EXISTS "${RunCMake_BINARY_DIR}/tidy-fixes.yaml")
  string(APPEND RunCMake_TEST_FAILED "Expected ${RunCMake_BINARY_DIR}/tidy-fixes.yaml not to exist but it does")
endif()
