if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/empty.cpp)
  set(RunCMake_TEST_FAILED "Policy CMP0154 set to OLD but using new behavior compiled sources.")
endif()
