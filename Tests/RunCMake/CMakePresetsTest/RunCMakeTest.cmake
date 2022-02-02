include(RunCMake)

# Presets do not support legacy VS generator name architecture suffix.
if(RunCMake_GENERATOR MATCHES "^(Visual Studio [0-9]+ [0-9]+) ")
  set(RunCMake_GENERATOR "${CMAKE_MATCH_1}")
endif()

function(run_cmake_test_presets name CMakePresetsTest_CONFIGURE_PRESETS CMakePresetsTest_BUILD_PRESETS CMakePresetsTest_TEST_PRESETS)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_TEST_SOURCE_DIR}/build")
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")

  set(RunCMake_TEST_NO_CLEAN TRUE)

  file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  set(CASE_NAME "${name}")
  set(CASE_SOURCE_DIR "${RunCMake_SOURCE_DIR}")
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in" "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" @ONLY)

  if(NOT CMakePresetsTest_FILE)
    set(CMakePresetsTest_FILE "${RunCMake_SOURCE_DIR}/${name}.json.in")
  endif()
  if(EXISTS "${CMakePresetsTest_FILE}")
    configure_file("${CMakePresetsTest_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json" @ONLY)
  endif()

  if(NOT CMakeUserPresets_FILE)
    set(CMakeUserPresets_FILE "${RunCMake_SOURCE_DIR}/${name}User.json.in")
  endif()
  if(EXISTS "${CMakeUserPresets_FILE}")
    configure_file("${CMakeUserPresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json" @ONLY)
  endif()

  foreach(ASSET ${CMakePresetsTest_ASSETS})
    configure_file("${RunCMake_SOURCE_DIR}/${ASSET}" "${RunCMake_TEST_SOURCE_DIR}" COPYONLY)
  endforeach()

  if (NOT CMakePresetsTest_NO_CONFIGURE)
    foreach(CONFIGURE_PRESET ${CMakePresetsTest_CONFIGURE_PRESETS})
      run_cmake_command("${name}-configure-${CONFIGURE_PRESET}"
        "${CMAKE_COMMAND}" "--preset" "${CONFIGURE_PRESET}")
    endforeach()
  endif()

  if (NOT CMakePresetsTest_NO_BUILD)
    foreach(BUILD_PRESET ${CMakePresetsTest_BUILD_PRESETS})
      run_cmake_command("${name}-build-${BUILD_PRESET}"
        "${CMAKE_COMMAND}" "--build" "--preset" "${BUILD_PRESET}")
    endforeach()
  endif()

  set(eq 0)
  foreach(TEST_PRESET ${CMakePresetsTest_TEST_PRESETS})
    if (EXISTS "${RunCMake_SOURCE_DIR}/${name}-test-${TEST_PRESET}-check.cmake")
      set(RunCMake-check-file "${name}-test-${TEST_PRESET}-check.cmake")
    else()
      set(RunCMake-check-file "check.cmake")
    endif()

    if(eq)
      run_cmake_command(${name}-test-${TEST_PRESET}
        ${CMAKE_CTEST_COMMAND} "--preset=${TEST_PRESET}" ${ARGN})
      set(eq 0)
    else()
      run_cmake_command(${name}-test-${TEST_PRESET}
        ${CMAKE_CTEST_COMMAND} "--preset" "${TEST_PRESET}" ${ARGN})
      set(eq 1)
    endif()
  endforeach()
endfunction()

set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
set(CMakePresetsTest_NO_BUILD 1)

set(CMakePresetsTest_ASSETS "Good-indexFile.txt")
set(GoodTestPresets
  "minimal;defaults;noEnvironment;withEnvironment"
  "config-debug;config-release"
  "exclude;index;indexFile;showOnly")
run_cmake_test_presets(Good
                       "default"
                       ""
                       "${GoodTestPresets}")
unset(CMakePresetsTest_ASSETS)

run_cmake_test_presets(InvalidConfigurePreset "default" "" "badConfigurePreset")

set(CMakePresetsTest_NO_CONFIGURE 1)
set(CMakePresetsTest_FILE "${RunCMake_SOURCE_DIR}/Good.json.in")
run_cmake_test_presets(ListPresets "" "" "x" "--list-presets")

set(CMakePresetsTest_FILE "${RunCMake_SOURCE_DIR}/Condition.json.in")
run_cmake_test_presets(ConditionListPresets "" "" "x" "--list-presets")
unset(CMakePresetsTest_NO_CONFIGURE)
run_cmake_test_presets(ConditionRunTests "default" "" "enabled;disabled")
set(CMakePresetsTest_NO_CONFIGURE 1)
unset(CMakePresetsTest_FILE)

run_cmake_test_presets(NoConfigurePreset "" "" "noConfigurePreset")
run_cmake_test_presets(NoTestsAction "default" "" "noTestsAction")
run_cmake_test_presets(Invalid "" "" "hidden;vendorMacro")

set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_test_presets(PresetsUnsupported "" "" "x")
run_cmake_test_presets(ConditionFuture "" "" "x")
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_test_presets(ConfigurePresetUnreachable "" "" "x")
set(CMakePresetsTest_NO_CONFIGURE 0)

set(CMakePresetsTest_NO_BUILD 0)
