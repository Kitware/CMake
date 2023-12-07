if (EXISTS ${RunCMake_TEST_BINARY_DIR}/private.h)
  set(RunCMake_TEST_FAILED "Private header generated before compilation.")
endif()
