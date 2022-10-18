include("${CMAKE_CURRENT_LIST_DIR}/check.cmake")
if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/default/output.log")
  string(APPEND RunCMake_TEST_FAILED "Expected ${RunCMake_TEST_BINARY_DIR}/default/output.log to exist but it does not\n")
endif()
