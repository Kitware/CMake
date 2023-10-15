if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/private.h)
  set(RunCMake_TEST_FAILED "Private header should be generated for target compilation")
endif()
