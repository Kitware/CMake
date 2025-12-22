include(RunCMake)
include(RunCTest)

function(run_ctest_TimeoutAfterMatch CASE_NAME)
  set(CASE_PROPERTY_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_TimeoutAfterMatch(MissingArg1 "\"-Darg2=Test started\"")
run_ctest_TimeoutAfterMatch(MissingArg2 "\"-Darg1=2\"")
run_ctest_TimeoutAfterMatch(ShouldTimeout "\"-Darg1=1\" \"-Darg2=Test started\"")
run_ctest_TimeoutAfterMatch(ShouldPass "\"-Darg1=15\" \"-Darg2=Test started\"")

function(run_ctest_cli_TimeoutAfterMatch CASE_NAME)
  set(test_name "${CASE_NAME}")

  set(RunCMake_TEST_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/${test_name}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${test_name}-build")

  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
  else ()
    set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
  endif ()
  list(APPEND RunCMake_TEST_OPTIONS
    ${ARGN})
  run_cmake("${test_name}")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command("${test_name}-build" "${CMAKE_COMMAND}" --build . --config Debug)
  run_cmake_command("${test_name}-test" "${CMAKE_CTEST_COMMAND}" -C Debug -VV)
endfunction()

run_ctest_cli_TimeoutAfterMatch(ShouldTimeoutNoBaseTimeout "-Dno_timeout=1" "-Darg1=1" "-Darg2=Test started")
