set(expected_file "${RunCMake_TEST_BINARY_DIR}/subdir/target_in_subdir.out")
if(NOT EXISTS "${expected_file}")
  set(RunCMake_TEST_FAILED "Expected build output file not found:\n ${expected_file}")
endif()
