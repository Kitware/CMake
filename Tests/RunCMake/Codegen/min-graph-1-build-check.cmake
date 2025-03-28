set(filename "${RunCMake_TEST_BINARY_DIR}/generated.h")
if (NOT EXISTS "${filename}")
  set(RunCMake_TEST_FAILED "expected file NOT created:\n ${filename}")
  return()
endif()

# foobar should be built since it was needed
# by the code generation
set(filename "${RunCMake_TEST_BINARY_DIR}/foobar.txt")
if (NOT EXISTS "${filename}")
  set(RunCMake_TEST_FAILED "expected file NOT created:\n ${filename}")
  return()
endif()
