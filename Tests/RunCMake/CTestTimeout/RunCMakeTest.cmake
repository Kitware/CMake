include(RunCTest)

if(NOT TIMEOUT)
  # Give the process time to load and start running.
  set(TIMEOUT 4)
endif()

function(run_ctest_timeout CASE_NAME)
  configure_file(${RunCMake_SOURCE_DIR}/TestTimeout.c
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/TestTimeout.c COPYONLY)
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_timeout(Basic)
