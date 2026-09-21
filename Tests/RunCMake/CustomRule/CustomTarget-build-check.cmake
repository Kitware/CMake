
if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/file.i")
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/file.i is missing.")
endif()
