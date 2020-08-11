set(expected "${RunCMake_TEST_BINARY_DIR}/SubDir/out.txt")
if(NOT EXISTS "${expected}")
  set(RunCMake_TEST_FAILED "Expected file not created:\n  ${expected}")
endif()
