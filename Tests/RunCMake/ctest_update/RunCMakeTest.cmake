include(RunCTest)
set(CASE_CTEST_UPDATE_ARGS "")
function(run_ctest_update CASE_NAME)
  set(CASE_CTEST_UPDATE_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_update(TestQuiet QUIET)

function(run_TestChangeId)
  set(CASE_TEST_PREFIX_CODE [[
    set(CTEST_CHANGE_ID "<>1")
  ]])

  run_ctest(TestChangeId)
endfunction()
run_TestChangeId()
