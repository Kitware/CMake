set(filename "${RunCMake_TEST_BINARY_DIR}/generated.h")
if(NOT EXISTS "${filename}")
  string(APPEND RunCMake_TEST_FAILED "expected file NOT created:\n ${filename}\n")
endif()

set(filename "${RunCMake_TEST_BINARY_DIR}/SubDir/generated.h")
if(NOT EXISTS "${filename}")
  string(APPEND RunCMake_TEST_FAILED "expected file NOT created:\n ${filename}\n")
endif()
