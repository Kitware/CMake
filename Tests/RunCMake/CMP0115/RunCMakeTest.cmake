include(RunCMake)

function(run_cmp0115 status)
  if(NOT status STREQUAL "WARN")
    set(RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0115=${status})
  endif()
  run_cmake(CMP0115-${status})
endfunction()

run_cmp0115(OLD)
run_cmp0115(WARN)
run_cmp0115(NEW)
