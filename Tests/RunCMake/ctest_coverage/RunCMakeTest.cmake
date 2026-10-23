include(RunCTest)

set(CASE_CTEST_COVERAGE_ARGS "")

function(run_ctest_coverage CASE_NAME)
  set(CASE_CTEST_COVERAGE_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_coverage(CoverageQuiet QUIET)

block()
  set(CASE SetOutputDir)
  set(DEFAULT_OUTPUT_DIR "${RunCMake_BINARY_DIR}/${CASE}-build/Testing")
  set(OUTPUT_DIR "${RunCMake_BINARY_DIR}/${CASE}-build/CTestCoverageOutputDir")
  run_ctest_coverage(${CASE} QUIET)
endblock()
