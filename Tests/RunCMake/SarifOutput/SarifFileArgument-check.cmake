# Make sure the output exists
if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/test_cmake_run.sarif")
  message(FATAL_ERROR "SARIF file not generated in the expected location")
endif()
