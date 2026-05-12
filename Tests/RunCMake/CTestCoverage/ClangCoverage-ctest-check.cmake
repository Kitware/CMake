if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/ClangCoverage.profdata")
  string(APPEND RunCMake_TEST_FAILED "ClangCoverage.profdata not found\n")
endif()
