include(RunCMake)

function(run_GoogleTest)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/GoogleTest-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(RunCMake_GENERATOR MATCHES "Make|Ninja")
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(GoogleTest)
  run_cmake_command(GoogleTest-build
    ${CMAKE_COMMAND}
    --build .
    --config Debug
  )
  run_cmake_command(GoogleTest-test
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -L TEST
    --no-label-summary
  )
endfunction()

run_GoogleTest()
