include(RunCTest)

# Isolate our ctest runs from external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})

function(run_ctest_test CASE_NAME)
  set(CASE_CTEST_FIXTURES_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

#------------------------------------------------------------
# CMake configure will pass
#------------------------------------------------------------
run_ctest_test(one      INCLUDE one)
run_ctest_test(two      INCLUDE two)
run_ctest_test(three    INCLUDE three)
run_ctest_test(setupFoo INCLUDE setupFoo)
run_ctest_test(wontRun  INCLUDE wontRun)
run_ctest_test(unused   INCLUDE Unused)

#------------------------------------------------------------
# CMake configure will fail due to cyclic test dependencies
#------------------------------------------------------------
set(CASE_CMAKELISTS_CYCLIC_CODE [[
    set_tests_properties(cyclicSetup PROPERTIES
                         FIXTURES_SETUP    "Foo"
                         FIXTURES_REQUIRED "Foo")
]])
run_ctest(cyclicSetup)

set(CASE_CMAKELISTS_CYCLIC_CODE [[
    set_tests_properties(cyclicCleanup PROPERTIES
                         FIXTURES_CLEANUP  "Foo"
                         FIXTURES_REQUIRED "Foo")
]])
run_ctest(cyclicCleanup)
