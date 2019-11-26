set(last_test_log "${RunCMake_TEST_BINARY_DIR}/Testing/Temporary/LastTest.log")
if(EXISTS "${last_test_log}")
  file(READ "${last_test_log}" last_test_log_content)
  string(REGEX REPLACE "\n+$" "" last_test_log_content "${last_test_log_content}")
  if(NOT last_test_log_content MATCHES "
Test Pass Reason:
Skip regular expression found in output. Regex=[[]test1]")
    string(REPLACE "\n" "\n  " last_test_log_content "  ${last_test_log_content}")
    set(RunCMake_TEST_FAILED "LastTest.log does not have expected content:\n${last_test_log_content}")
  endif()
else()
  set(RunCMake_TEST_FAILED "LastTest.log missing:\n ${last_test_log}")
endif()
