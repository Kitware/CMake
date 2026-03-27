include(RunCMake)

# Isolate our ctest runs from external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})

if(NOT RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
endif()

function(run_case CASE)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${CASE}-build)
  run_cmake(${CASE})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${CASE}-build ${CMAKE_COMMAND} --build .  --config Debug)
  run_cmake_command(${CASE}-test ${CMAKE_CTEST_COMMAND} -C Debug)
endfunction()

run_case(Discovery)
