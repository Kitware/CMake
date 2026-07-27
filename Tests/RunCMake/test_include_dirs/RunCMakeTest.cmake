include(RunCMake)

# Isolate our ctest runs from external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})

function(run_TID)
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TID-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(TID)
  run_cmake_command(TID-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(TID-test ${CMAKE_CTEST_COMMAND} -C Debug)
endfunction()

run_TID()

function(run_TIDGenex)
  # Generator expressions in TEST_INCLUDE_FILE(S) include paths.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TIDGenex-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif()
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(TIDGenex)
  run_cmake_command(TIDGenex-ctest-noC ${CMAKE_CTEST_COMMAND})
  run_cmake_command(TIDGenex-ctest-Debug ${CMAKE_CTEST_COMMAND} -C Debug)
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    run_cmake_command(TIDGenex-ctest-Release ${CMAKE_CTEST_COMMAND} -C Release)
  endif()
endfunction()

run_TIDGenex()

# Serialization/quoting rules for genex-evaluated include paths (configure only).
run_cmake(TIDGenexQuoting)

# An unknown generator expression must fail at generate time.
run_cmake(TIDGenexBadGenex)
