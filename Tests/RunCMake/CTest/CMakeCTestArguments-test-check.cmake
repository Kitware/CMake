set(log "${RunCMake_TEST_BINARY_DIR}/output-log.txt")
if(NOT EXISTS "${log}")
  set(RunCMake_TEST_FAILED "The expected output log file is missing:\n  ${log}")
endif()
