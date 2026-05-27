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

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/ConfigurePresetVar")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
  @ONLY)
set(CASE_TEST_PREFIX_CODE [[set(CTEST_CONFIGURE_PRESET "my-preset")]])
run_ctest(ConfigurePresetVar)
unset(CASE_TEST_PREFIX_CODE)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/ConfigurePresetGenericVar")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
  @ONLY)
set(CASE_TEST_PREFIX_CODE [[set(CTEST_PRESET "my-preset")]])
run_ctest(ConfigurePresetGenericVar)
unset(CASE_TEST_PREFIX_CODE)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/ConfigurePresetFromFile")
set(custom_presets_file "${RunCMake_BINARY_DIR}/ConfigurePresetFromFile/custom-presets.json")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${custom_presets_file}"
  @ONLY)
run_ctest_configure(ConfigurePresetFromFile PRESET my-preset PRESETS_FILE "${custom_presets_file}")
unset(custom_presets_file)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/ConfigurePresetFromFileVar")
set(custom_presets_file "${RunCMake_BINARY_DIR}/ConfigurePresetFromFileVar/custom-presets.json")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${custom_presets_file}"
  @ONLY)
set(CASE_TEST_PREFIX_CODE
"set(CTEST_CONFIGURE_PRESET \"my-preset\")
set(CTEST_PRESETS_FILE \"${custom_presets_file}\")")
run_ctest(ConfigurePresetFromFileVar)
unset(CASE_TEST_PREFIX_CODE)
unset(custom_presets_file)
unset(RunCMake_TEST_SOURCE_DIR)
