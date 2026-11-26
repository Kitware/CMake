file(STRINGS "${RunCMake_TEST_BINARY_DIR}/Makefile" shell REGEX "^SHELL = ")
if(NOT shell STREQUAL "SHELL = ${expect_shell}")
  set(RunCMake_TEST_FAILED "Expected:\n  SHELL = ${expect_shell}\nActual:\n  ${shell}")
endif()
