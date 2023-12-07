if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/public.h)
  set(RunCMake_TEST_FAILED "Public header did not generate before compilation.")
endif()
