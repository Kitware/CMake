if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/private.h)
  set(RunCMake_TEST_FAILED "Policy CMP0154 set to OLD but using new behavior private headers.")
endif()
