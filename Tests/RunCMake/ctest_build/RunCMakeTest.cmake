include(RunCTest)

set(LANG NONE)
set(CASE_CTEST_BUILD_ARGS "")
set(RunCMake_USE_LAUNCHERS TRUE)
set(RunCMake_USE_CUSTOM_BUILD_COMMAND FALSE)

function(run_ctest_build CASE_NAME)
  set(CASE_CTEST_BUILD_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_build(BuildQuiet QUIET)
run_ctest_build(ParallelLevel PARALLEL_LEVEL 1)

block()
  set(LANG CXX)
  configure_file("${RunCMake_SOURCE_DIR}/BuildFailure.cxx" "${RunCMake_BINARY_DIR}/BuildFailure/BuildFailure.cxx" COPYONLY)
  set(CASE_CMAKELISTS_SUFFIX_CODE [=[
    add_executable(BuildFailure BuildFailure.cxx)
  ]=])
  set(CASE_CMAKELISTS_PREFIX_CODE [[
if(NOT CTEST_USE_LAUNCHERS)
  message(FATAL_ERROR "CTEST_USE_LAUNCHERS not set")
endif()
]])
  set(CASE_TEST_PREFIX_CODE "")
  set(CASE_TEST_SUFFIX_CODE [[
if (ctest_build_return_value)
  message("ctest_build returned non-zero")
else()
  message("ctest_build returned zero")
endif()
]])
  run_ctest(BuildFailure)
endblock()

function(run_BuildChangeId)
  set(CASE_TEST_PREFIX_CODE [[
    set(CTEST_CHANGE_ID "<>1")
  ]])

  run_ctest(BuildChangeId)
endfunction()
run_BuildChangeId()

function(run_SubdirTarget)
  set(CASE_CMAKELISTS_SUFFIX_CODE [=[
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/subdir/CMakeLists.txt [[
add_custom_target(target_in_subdir COMMAND ${CMAKE_COMMAND} -E touch target_in_subdir.out VERBATIM)
]])
add_subdirectory(subdir)
]=])
  set(CASE_CTEST_BUILD_ARGS TARGET target_in_subdir)
  run_ctest(SubdirTarget)
endfunction()
run_SubdirTarget()

set(RunCMake_USE_CUSTOM_BUILD_COMMAND TRUE)
set(RunCMake_BUILD_COMMAND "${FAKE_BUILD_COMMAND_EXE}")
run_ctest(BuildCommandFailure)
unset(RunCMake_BUILD_COMMAND)

set(RunCMake_USE_CUSTOM_BUILD_COMMAND TRUE)
set(RunCMake_BUILD_COMMAND "${FAKE_BUILD_COMMAND_EXE}")
run_ctest(BuildDirectories)
unset(RunCMake_BUILD_COMMAND)

set(RunCMake_USE_LAUNCHERS FALSE)
set(RunCMake_BUILD_COMMAND "${COLOR_WARNING}")
run_ctest(IgnoreColor)
unset(RunCMake_BUILD_COMMAND)

set(RunCMake_USE_CUSTOM_BUILD_COMMAND FALSE)
set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/BuildPreset")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
  @ONLY)
run_ctest_build(BuildPreset PRESET my-build-preset)
unset(RunCMake_TEST_SOURCE_DIR)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/BuildPresetVar")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
  @ONLY)
set(CASE_TEST_PREFIX_CODE [[set(CTEST_BUILD_PRESET "my-build-preset")]])
run_ctest(BuildPresetVar)
unset(CASE_TEST_PREFIX_CODE)
unset(RunCMake_TEST_SOURCE_DIR)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/BuildPresetGenericVar")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
  @ONLY)
set(CASE_TEST_PREFIX_CODE [[set(CTEST_PRESET "my-build-preset")]])
run_ctest(BuildPresetGenericVar)
unset(CASE_TEST_PREFIX_CODE)
unset(RunCMake_TEST_SOURCE_DIR)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/BuildPresetFromFile")
set(custom_presets_file
  "${RunCMake_BINARY_DIR}/BuildPresetFromFile/custom-presets.json")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${custom_presets_file}"
  @ONLY)
run_ctest_build(BuildPresetFromFile
  PRESET my-build-preset
  PRESETS_FILE "${custom_presets_file}")
unset(RunCMake_TEST_SOURCE_DIR)
unset(custom_presets_file)

set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/BuildPresetFromFileVar")
set(custom_presets_file
  "${RunCMake_BINARY_DIR}/BuildPresetFromFileVar/custom-presets.json")
configure_file(
  "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
  "${custom_presets_file}"
  @ONLY)
set(CASE_TEST_PREFIX_CODE
"set(CTEST_BUILD_PRESET \"my-build-preset\")
set(CTEST_PRESETS_FILE \"${custom_presets_file}\")")
run_ctest(BuildPresetFromFileVar)
unset(CASE_TEST_PREFIX_CODE)
unset(RunCMake_TEST_SOURCE_DIR)
unset(custom_presets_file)

run_ctest_build(BuildPresetBadFile
  PRESET my-build-preset
  PRESETS_FILE /nonexistent/path/presets.json)

# Helper for tests that verify -D variables reach ctest_build() in -M/-T mode.
# Accepts a case name followed by the -D arguments to pass to ctest.
# Sets up a source directory with CMakePresets.json and a binary directory with
# DartConfiguration.tcl, then runs ctest with -M Experimental -T Build -V.
function(run_build_cli_var_test CASE_NAME)
  set(case_source_dir "${RunCMake_BINARY_DIR}/${CASE_NAME}")
  set(case_binary_dir "${RunCMake_BINARY_DIR}/${CASE_NAME}-build")
  file(MAKE_DIRECTORY "${case_source_dir}")
  configure_file(
    "${RunCMake_SOURCE_DIR}/CMakePresets.json.in"
    "${case_source_dir}/CMakePresets.json"
    @ONLY)
  file(REMOVE_RECURSE "${case_binary_dir}")
  file(MAKE_DIRECTORY "${case_binary_dir}")
  file(WRITE "${case_binary_dir}/DartConfiguration.tcl"
    "BuildDirectory: ${case_binary_dir}\n"
    "SourceDirectory: ${case_source_dir}\n"
    "MakeCommand: \"${CMAKE_COMMAND}\" --build \"${case_binary_dir}\"\n")
  set(RunCMake_TEST_SOURCE_DIR "${case_source_dir}")
  set(RunCMake_TEST_BINARY_DIR "${case_binary_dir}")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${CASE_NAME}
    ${CMAKE_CTEST_COMMAND}
    -M Experimental
    ${ARGN}
    -T Build
    -V)
endfunction()

# Verify that CTEST_BUILD_PRESET passed via -D on the command line reaches
# ctest_build() when ctest is run with -M/-T.
run_build_cli_var_test(BuildPresetCLIVar
  -D "CTEST_BUILD_PRESET=my-build-preset")

# Verify that CTEST_BUILD_CONFIGURATION passed via -D on the command line
# reaches ctest_build() when ctest is run with -M/-T.
run_build_cli_var_test(BuildConfigurationCLIVar
  -D "CTEST_BUILD_PRESET=my-build-preset"
  -D "CTEST_BUILD_CONFIGURATION=my-config")

# Verify that CTEST_CONFIGURATION_TYPE passed via -D on the command line
# reaches ctest_build() when ctest is run with -M/-T.
run_build_cli_var_test(ConfigurationTypeCLIVar
  -D "CTEST_BUILD_PRESET=my-build-preset"
  -D "CTEST_CONFIGURATION_TYPE=my-type")

set(RunCMake_USE_CUSTOM_BUILD_COMMAND FALSE)
if(RunCMake_GENERATOR MATCHES "Ninja")
  function(run_NinjaLauncherSingleBuildFailure)
    set(LANG C)
    set(RunCMake_USE_LAUNCHERS TRUE)
    set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/NinjaLauncherSingleBuildFailure")
    configure_file("${RunCMake_SOURCE_DIR}/error.c" "${RunCMake_TEST_SOURCE_DIR}/error.c" COPYONLY)
    set(CASE_CMAKELISTS_SUFFIX_CODE [=[
    add_executable(error error.c)
  ]=])
    run_ctest(NinjaLauncherSingleBuildFailure)
  endfunction()
  run_NinjaLauncherSingleBuildFailure()
endif()
