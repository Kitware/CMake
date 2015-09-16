include(RunCMake)

function(run_BuildDepends CASE)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(RunCMake_GENERATOR MATCHES "Make|Ninja")
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  include(${RunCMake_SOURCE_DIR}/${CASE}.step1.cmake OPTIONAL)
  run_cmake(${CASE})
  set(RunCMake-check-file check.cmake)
  set(check_step 1)
  run_cmake_command(${CASE}-build1 ${CMAKE_COMMAND} --build . --config Debug)
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.125) # handle 1s resolution
  include(${RunCMake_SOURCE_DIR}/${CASE}.step2.cmake OPTIONAL)
  set(check_step 2)
  run_cmake_command(${CASE}-build2 ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_BuildDepends(C-Exe)
