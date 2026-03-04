include(RunCMake)

# Isolate our ctest runs from external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})

if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
endif()

block()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/basic-build")
  run_cmake(basic)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(basic-build ${CMAKE_COMMAND} --build . --config Debug)

  run_cmake_command(PROPERTY ${CMAKE_CTEST_COMMAND} -C Debug -N -L LBL1)

  set(ENV{TEST_LABEL} "FOO")
  run_cmake_command(ENVIRONMENT ${CMAKE_CTEST_COMMAND} -C Debug -R "Env.")

  run_cmake_command(COMMAND_EXPAND_LISTS
    ${CMAKE_CTEST_COMMAND} -C Debug -N -R "ExpandLists.")
endblock()

function(run_case CASE)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE}-build)
  run_cmake(${CASE})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${CASE}-build ${CMAKE_COMMAND} --build .  --config Debug)
  run_cmake_command(${CASE}-test ${CMAKE_CTEST_COMMAND} -C Debug)
endfunction()

run_case(bad-command)
run_case(bad-regex)
run_case(discovery-failure)
run_case(discovery-timeout)
