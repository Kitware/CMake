if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/empty.cpp)
  set(RunCMake_TEST_FAILED "Compiled source did not generate.")
endif()
