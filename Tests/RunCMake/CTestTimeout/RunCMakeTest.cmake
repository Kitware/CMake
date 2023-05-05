include(RunCTest)

if(NOT DEFINED TIMEOUT)
  # Give the process time to load and start running.
  set(TIMEOUT 4)
endif()

function(run_ctest_timeout CASE_NAME)
  configure_file(${RunCMake_SOURCE_DIR}/TestTimeout.c
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/TestTimeout.c COPYONLY)
  run_ctest(${CASE_NAME} ${ARGN})
endfunction()

run_ctest_timeout(Basic)

if(UNIX)
  string(CONCAT CASE_CMAKELISTS_SUFFIX_CODE [[
    target_compile_definitions(TestTimeout PRIVATE FORK)
]])
  run_ctest_timeout(Fork)
  unset(CASE_CMAKELISTS_SUFFIX_CODE)
endif()

block()
  # An explicit zero TIMEOUT test property means "no timeout".
  set(TIMEOUT 0)
  # The test sleeps for 4 seconds longer than the TIMEOUT value.
  # Set a default timeout to less than that so that the test will
  # timeout if the zero TIMEOUT does not suppress it.
  run_ctest_timeout(ZeroOverridesFlag --timeout 2)
  set(CASE_TEST_PREFIX_CODE "set(CTEST_TEST_TIMEOUT 2)")
  run_ctest_timeout(ZeroOverridesVar)
endblock()
