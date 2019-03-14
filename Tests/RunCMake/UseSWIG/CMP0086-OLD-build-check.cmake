
if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/example.py")
  set (RunCMake_TEST_FAILED "Not found expected file: '${RunCMake_TEST_BINARY_DIR}/example.py'.")
endif()
