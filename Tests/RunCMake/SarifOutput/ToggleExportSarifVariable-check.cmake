include("${CMAKE_CURRENT_LIST_DIR}/check-sarif.cmake")

# This test should produce the same output as GenerateSarifResults
check_sarif_output("${RunCMake_TEST_BINARY_DIR}/.cmake/sarif/cmake.sarif"
  "${CMAKE_CURRENT_LIST_DIR}/GenerateSarifResults-expected.sarif")
