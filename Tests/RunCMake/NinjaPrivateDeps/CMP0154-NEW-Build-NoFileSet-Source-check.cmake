if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/none.cpp)
  set(RunCMake_TEST_FAILED "Private source dependency used for target without filesets.")
endif()
