include(RunCMake)

# Presets do not support legacy VS generator name architecture suffix.
if(RunCMake_GENERATOR MATCHES "^(Visual Studio [0-9]+ [0-9]+) ")
  set(RunCMake_GENERATOR "${CMAKE_MATCH_1}")
endif()

function(run_cmake_build_presets name CMakePresetsBuild_CONFIGURE_PRESETS CMakePresetsBuild_BUILD_PRESETS)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_TEST_SOURCE_DIR}/build")
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")

  set(RunCMake_TEST_NO_CLEAN TRUE)

  file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  set(CASE_NAME "${name}")
  set(CASE_SOURCE_DIR "${RunCMake_SOURCE_DIR}")
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in" "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" @ONLY)

  if(NOT CMakePresetsBuild_FILE)
    set(CMakePresetsBuild_FILE "${RunCMake_SOURCE_DIR}/${name}.json.in")
  endif()
  if(EXISTS "${CMakePresetsBuild_FILE}")
    configure_file("${CMakePresetsBuild_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json" @ONLY)
  endif()

  if(NOT CMakeUserPresets_FILE)
    set(CMakeUserPresets_FILE "${RunCMake_SOURCE_DIR}/${name}User.json.in")
  endif()
  if(EXISTS "${CMakeUserPresets_FILE}")
    configure_file("${CMakeUserPresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json" @ONLY)
  endif()

  if (NOT CMakePresetsBuild_BUILD_ONLY)
    foreach(CONFIGURE_PRESET ${CMakePresetsBuild_CONFIGURE_PRESETS})
      run_cmake_command("${name}-configure-${CONFIGURE_PRESET}"
        "${CMAKE_COMMAND}" "--preset" "${CONFIGURE_PRESET}")
    endforeach()
  endif()

  set(eq 0)
  foreach(BUILD_PRESET ${CMakePresetsBuild_BUILD_PRESETS})
    if (EXISTS "${RunCMake_SOURCE_DIR}/${name}-build-${BUILD_PRESET}-check.cmake")
      set(RunCMake-check-file "${name}-build-${BUILD_PRESET}-check.cmake")
    else()
      set(RunCMake-check-file "check.cmake")
    endif()

    if(eq)
      run_cmake_command(${name}-build-${BUILD_PRESET}
        ${CMAKE_COMMAND} "--build" "--preset=${BUILD_PRESET}" ${ARGN})
      set(eq 0)
    else()
      run_cmake_command(${name}-build-${BUILD_PRESET}
        ${CMAKE_COMMAND} "--build" "--preset" "${BUILD_PRESET}" ${ARGN})
      set(eq 1)
    endif()
  endforeach()
endfunction()

set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)

if(RunCMake_GENERATOR MATCHES "NMake|Borland|Watcom")
  set(Good_json_jobs [[]])
elseif(RunCMake_GENERATOR MATCHES "Make" AND CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(Good_json_jobs [["jobs": 1,]])
else()
  set(Good_json_jobs [["jobs": 0,]])
endif()

run_cmake_build_presets(Good "default;other" "build-other;withEnvironment;noEnvironment;macros;vendorObject;singleTarget;initResolve")
run_cmake_build_presets(InvalidConfigurePreset "default" "badConfigurePreset")
run_cmake_build_presets(Condition "default" "enabled;disabled")

set(CMakePresetsBuild_BUILD_ONLY 1)
run_cmake_build_presets(ListPresets "x" "x" "--list-presets")
run_cmake_build_presets(NoConfigurePreset "x" "noConfigurePreset")
run_cmake_build_presets(Invalid "x" "hidden;vendorMacro")

set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_build_presets(PresetsUnsupported "x" "x")
run_cmake_build_presets(ConditionFuture "x" "conditionFuture")
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)

run_cmake_build_presets(ConfigurePresetUnreachable "x" "x")
set(CMakePresetsBuild_BUILD_ONLY 0)
