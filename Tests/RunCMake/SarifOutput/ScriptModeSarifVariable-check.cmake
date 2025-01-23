# Script mode should ignore the SARIF project variable and export nothing
if (EXISTS "${RunCMake_TEST_BINARY_DIR}/.cmake/sarif/cmake.sarif")
  message(FATAL_ERROR "SARIF file should not have been generated in script mode")
endif()
