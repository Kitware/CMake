include(RunCTest)
set(CASE_CTEST_UPDATE_ARGS "")
function(run_ctest_update CASE_NAME)
  set(CASE_CTEST_UPDATE_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_update(UpdateQuiet QUIET)

function(run_UpdateChangeId)
  set(CASE_TEST_PREFIX_CODE [[
    set(CTEST_CHANGE_ID "<>1")
  ]])

  run_ctest(UpdateChangeId)
endfunction()
run_UpdateChangeId()

function(run_UpdateVersionOverride)
  set(CASE_TEST_PREFIX_CODE [[
    set(CTEST_UPDATE_VERSION_OVERRIDE "qwertyuiop")
  ]])
  run_ctest(UpdateVersionOverride)
endfunction()
run_UpdateVersionOverride()
