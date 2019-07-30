if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/output")
  message(FATAL_ERROR "Failed to create output: ${RunCMake_TEST_BINARY_DIR}/output")
endif()
