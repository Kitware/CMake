if(NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/custom-output.txt)
  set(RunCMake_TEST_FAILED "Building 'test' target did not build 'all' target:\n ${RunCMake_TEST_BINARY_DIR}/custom-output.txt")
endif()
