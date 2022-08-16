include(RunCTest)

# Isolate our ctest runs from external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})

set(CASE_SOURCE_DIR "${RunCMake_SOURCE_DIR}")
set(RunCTest_VERBOSE_FLAG "-VV")

run_ctest(ENVIRONMENT_MODIFICATION-invalid-op)
run_ctest(ENVIRONMENT_MODIFICATION-no-colon)
run_ctest(ENVIRONMENT_MODIFICATION-no-equals)

set(ENV{CTEST_TEST_VAR} set-via-system-environment)
run_ctest(ENVIRONMENT_MODIFICATION-reset-to-prop)
run_ctest(ENVIRONMENT_MODIFICATION-reset-to-system)
