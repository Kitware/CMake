include(RunCMake)

# Presets do not support legacy VS generator name architecture suffix.
if(RunCMake_GENERATOR MATCHES "^(Visual Studio [0-9]+ [0-9]+) ")
  set(RunCMake_GENERATOR "${CMAKE_MATCH_1}")
endif()

function(run_cmake_workflow_presets name)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/${name}")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_TEST_SOURCE_DIR}/build")
  set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")

  if(NOT RunCMake_TEST_NO_CLEAN)
    file(REMOVE_RECURSE "${RunCMake_TEST_SOURCE_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}")
  endif()

  set(RunCMake_TEST_NO_CLEAN TRUE)

  set(CASE_NAME "${name}")
  set(CASE_SOURCE_DIR "${RunCMake_SOURCE_DIR}")
  configure_file("${RunCMake_SOURCE_DIR}/CMakeLists.txt.in" "${RunCMake_TEST_SOURCE_DIR}/CMakeLists.txt" @ONLY)

  if(NOT CMakePresets_FILE)
    set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/${name}.json.in")
  endif()
  if(EXISTS "${CMakePresets_FILE}")
    configure_file("${CMakePresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json" @ONLY)
  endif()

  if(NOT CMakeUserPresets_FILE)
    set(CMakeUserPresets_FILE "${RunCMake_SOURCE_DIR}/${name}User.json.in")
  endif()
  if(EXISTS "${CMakeUserPresets_FILE}")
    configure_file("${CMakeUserPresets_FILE}" "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json" @ONLY)
  endif()

  set(_presets_file)
  if(CMakePresets_FILE_ARG)
    set(_presets_file "--presets-file=${RunCMake_TEST_SOURCE_DIR}/${CMakePresets_FILE_ARG}")
    configure_file("${RunCMake_SOURCE_DIR}/${CMakePresets_FILE_ARG}.in" "${RunCMake_TEST_SOURCE_DIR}/${CMakePresets_FILE_ARG}" @ONLY)
  endif()

  foreach(ASSET ${CMakePresets_ASSETS})
    configure_file("${RunCMake_SOURCE_DIR}/${ASSET}.in" "${RunCMake_TEST_SOURCE_DIR}/${ASSET}" @ONLY)
  endforeach()

  if(EXISTS "${RunCMake_SOURCE_DIR}/${name}-check.cmake")
    set(RunCMake-check-file "${name}-check.cmake")
  else()
    set(RunCMake-check-file "check.cmake")
  endif()

  if(CMakePresets_NO_PRESET)
    set(preset_arg)
  elseif(CMakePresets_DIRECT_ARG)
    set(preset_arg "${name}")
  elseif(eq)
    set(eq 0 PARENT_SCOPE)
    set(preset_arg "--preset=${name}")
  else()
    set(eq 1 PARENT_SCOPE)
    set(preset_arg "--preset" "${name}")
  endif()
  run_cmake_command("${name}" "${CMAKE_COMMAND}" "--workflow" ${preset_arg} ${_presets_file} ${ARGN})
endfunction()

set(CMakePresets_SCHEMA_EXPECTED_RESULT 1)
run_cmake_workflow_presets(UnsupportedVersion)
run_cmake_workflow_presets(WorkflowStepInvalidType)
set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
run_cmake_workflow_presets(NoWorkflowSteps)
run_cmake_workflow_presets(FirstStepNotConfigure)
run_cmake_workflow_presets(SecondStepConfigure)
run_cmake_workflow_presets(NonexistentStep)
run_cmake_workflow_presets(UnreachableStep)
run_cmake_workflow_presets(WorkflowStepHidden)
run_cmake_workflow_presets(WorkflowStepDisabled)
run_cmake_workflow_presets(WorkflowStepInvalidMacro)
run_cmake_workflow_presets(ConfigureStepMismatch)

set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/Good.json.in")
set(CMakeUserPresets_FILE "${RunCMake_SOURCE_DIR}/GoodUser.json.in")
set(CMakePresets_ASSETS cpack_staging.cmake)
run_cmake_workflow_presets(Good)
run_cmake_workflow_presets(GoodUser)
run_cmake_workflow_presets(BadExitCode)
unset(CMakePresets_FILE)
unset(CMakeUserPresets_FILE)
unset(CMakePresets_ASSETS)

set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/DoesNotExist.json.in")
set(CMakePresets_FILE_ARG "OtherCMakePresetsFile.json")
run_cmake_workflow_presets(OtherCMakePresetsFile)
run_cmake_workflow_presets(OtherCMakePresetsFileListPresets --list-presets)
unset(CMakePresets_FILE)
unset(CMakePresets_FILE_ARG)

run_cmake_workflow_presets(ListPresets --list-presets)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/ListPresets.json.in")
unset(ENV{CMAKE_PRESET_TEST_UNFULFILLED_ENV})
set(CMakePresets_NO_PRESET TRUE)
run_cmake_workflow_presets(ListAllPresets --list-presets)
run_cmake_workflow_presets(ListDefinedPresets --list-presets=defined)
unset(CMakePresets_NO_PRESET)
run_cmake_command(ListAllPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=all
)
run_cmake_command(ListDefinedPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=all-defined
)
set(RunCMake-stdout-file ListDefinedConfigurePresetTypes-stdout.txt)
run_cmake_command(ListDefinedConfigurePresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=defined
)
run_cmake_command(ListConfigureDefinedPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=configure-defined
)
unset(RunCMake-stdout-file)
run_cmake_command(ListBuildPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=build
)
run_cmake_command(ListTestPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=test
)
run_cmake_command(ListPackagePresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=package
)
run_cmake_command(ListWorkflowPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=workflow
)
set(RunCMake-stdout-file ListDefinedBuildPresetsCommand-stdout.txt)
run_cmake_command(ListBuildDefinedPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=build-defined
)
unset(RunCMake-stdout-file)
set(RunCMake-stdout-file ListDefinedTestPresetsCommand-stdout.txt)
run_cmake_command(ListTestDefinedPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=test-defined
)
unset(RunCMake-stdout-file)
set(RunCMake-stdout-file ListDefinedPackagePresetsCommand-stdout.txt)
run_cmake_command(ListPackageDefinedPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=package-defined
)
unset(RunCMake-stdout-file)
set(RunCMake-stdout-file ListDefinedPresets-stdout.txt)
run_cmake_command(ListWorkflowDefinedPresetTypes
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=workflow-defined
)
unset(RunCMake-stdout-file)
set(ListPresets_FILE
  "${RunCMake_BINARY_DIR}/ListAllPresets/CMakePresets.json")
run_cmake_command(ListBuildPresetsCommand
  ${CMAKE_COMMAND}
  --build
  "--presets-file=${ListPresets_FILE}"
  --list-presets
)
run_cmake_command(ListDefinedBuildPresetsCommand
  ${CMAKE_COMMAND}
  --build
  "--presets-file=${ListPresets_FILE}"
  --list-presets=defined
)
run_cmake_command(ListTestPresetsCommand
  ${CMAKE_CTEST_COMMAND}
  "--presets-file=${ListPresets_FILE}"
  --list-presets
)
run_cmake_command(ListDefinedTestPresetsCommand
  ${CMAKE_CTEST_COMMAND}
  "--presets-file=${ListPresets_FILE}"
  --list-presets=defined
)
run_cmake_command(ListPackagePresetsCommand
  ${CMAKE_CPACK_COMMAND}
  "--presets-file=${ListPresets_FILE}"
  --list-presets
)
run_cmake_command(ListDefinedPackagePresetsCommand
  ${CMAKE_CPACK_COMMAND}
  "--presets-file=${ListPresets_FILE}"
  --list-presets=defined
)
run_cmake_command(ListTestPresetsInvalidValue
  ${CMAKE_CTEST_COMMAND}
  --list-presets=invalid
)
run_cmake_command(ListPackagePresetsInvalidValue
  ${CMAKE_CPACK_COMMAND}
  --list-presets=invalid
)
set(ENV{CMAKE_PRESET_TEST_UNFULFILLED_ENV} "set_value")
run_cmake_command(ListAllPresetTypesConditionTrue
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=all
)
run_cmake_command(ListDefinedPresetTypesConditionTrue
  ${CMAKE_COMMAND}
  -S "${RunCMake_BINARY_DIR}/ListAllPresets"
  --list-presets=all-defined
)
unset(ENV{CMAKE_PRESET_TEST_UNFULFILLED_ENV})
unset(ListPresets_FILE)
unset(CMakePresets_FILE)

run_cmake_command(PresetsNoArg-workflow ${CMAKE_COMMAND} "--workflow" "--preset")
run_cmake_command(PresetsNoArgEq-workflow ${CMAKE_COMMAND} "--workflow" "--preset=")
run_cmake_command(PresetsFileNoArg-workflow ${CMAKE_COMMAND} "--workflow" "--presets-file")
run_cmake_workflow_presets(InvalidOption -DINVALID_OPTION)
run_cmake_workflow_presets(ListPresetsInvalidValue --list-presets=invalid)

set(RunCMake_TEST_NO_CLEAN TRUE)
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/Fresh")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/Fresh/build")
file(WRITE "${RunCMake_BINARY_DIR}/Fresh/build/CMakeCache.txt" "FRESH_CONFIGURE:BOOL=OFF\n")
run_cmake_workflow_presets(Fresh --fresh)
unset(RunCMake_TEST_NO_CLEAN)

set(CMakePresets_DIRECT_ARG TRUE)
set(CMakePresets_FILE "${RunCMake_SOURCE_DIR}/MultiplePresets.json.in")
run_cmake_workflow_presets(SinglePresetArg)
run_cmake_workflow_presets(RepeatedPresetArg --preset SinglePresetArg)
unset(CMakePresets_FILE)
unset(CMakePresets_DIRECT_ARG)
