if(EXISTS ${RunCMake_TEST_BINARY_DIR}/custom-output.txt)
  set(RunCMake_TEST_FAILED "Building 'test' target incorrectly built 'all' target.")
endif()
