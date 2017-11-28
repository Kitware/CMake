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
    --target fake_gtest
  )

  set(RunCMake_TEST_OUTPUT_MERGE 1)
  run_cmake_command(GoogleTest-timeout
    ${CMAKE_COMMAND}
    --build .
    --config Debug
    --target timeout_test
  )
  set(RunCMake_TEST_OUTPUT_MERGE 0)

  run_cmake_command(GoogleTest-test1
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -L TEST1
    --no-label-summary
  )

  run_cmake_command(GoogleTest-test2
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -L TEST2
    --no-label-summary
  )

  run_cmake_command(GoogleTest-test-missing
    ${CMAKE_CTEST_COMMAND}
    -C Debug
    -R timeout
    --no-label-summary
  )
endfunction()

run_GoogleTest()
