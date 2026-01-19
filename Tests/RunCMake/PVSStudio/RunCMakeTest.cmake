include(RunCMake)

set(RunCMake_TEST_OPTIONS "-DPSEUDO_PVS=${PSEUDO_PVS}")

function(run_pvs test)
  # Use a single build tree for tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test})
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${test})
  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build .)
endfunction()

run_pvs(pvs)
run_pvs(pvs-bad-arg)
