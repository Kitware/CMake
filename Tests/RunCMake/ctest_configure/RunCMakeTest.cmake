include(RunCTest)

set(CASE_CTEST_CONFIGURE_ARGS "")

function(run_ctest_configure CASE_NAME)
  set(CASE_CTEST_CONFIGURE_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_configure(ConfigureQuiet QUIET)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/ConfigurePreset")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
  @ONLY)
run_ctest_configure(ConfigurePreset PRESET my-preset)
