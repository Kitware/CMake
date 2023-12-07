if (EXISTS ${RunCMake_TEST_BINARY_DIR}/empty.cpp)
  set(RunCMake_TEST_FAILED "Compiled source generated before compilation of consumers.")
endif()
