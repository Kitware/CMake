if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/output")
  set(RunCMake_TEST_FAILED "Failed to create output:\n  ${RunCMake_TEST_BINARY_DIR}/output")
  return()
endif()
