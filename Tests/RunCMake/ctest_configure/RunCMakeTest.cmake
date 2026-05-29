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

# Verify that CTEST_PRESET passed via -D on the command line reaches
# ctest_configure() when ctest is run with -M/-T.
set(case_source_dir "${RunCMake_BINARY_DIR}/ConfigurePresetCLIVar")
set(case_binary_dir "${RunCMake_BINARY_DIR}/ConfigurePresetCLIVar-build")
set(CASE_NAME "ConfigurePresetCLIVar")
file(MAKE_DIRECTORY "${case_source_dir}")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakeLists.txt.in"
  "${case_source_dir}/CMakeLists.txt"
  @ONLY)
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${case_source_dir}/CMakePresets.json"
  @ONLY)
file(REMOVE_RECURSE "${case_binary_dir}")
file(MAKE_DIRECTORY "${case_binary_dir}")
file(WRITE "${case_binary_dir}/DartConfiguration.tcl"
  "BuildDirectory: ${case_binary_dir}\n"
  "SourceDirectory: ${case_source_dir}\n"
  "ConfigureCommand: \"${CMAKE_COMMAND}\" -S\"${case_source_dir}\" -B\"${case_binary_dir}\"\n")
set(RunCMake_TEST_SOURCE_DIR "${case_source_dir}")
set(RunCMake_TEST_BINARY_DIR "${case_binary_dir}")
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(ConfigurePresetCLIVar
  ${CMAKE_CTEST_COMMAND}
  -M Experimental
  -D "CTEST_PRESET=my-preset"
  -T Configure
  -V)
unset(RunCMake_TEST_SOURCE_DIR)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)
