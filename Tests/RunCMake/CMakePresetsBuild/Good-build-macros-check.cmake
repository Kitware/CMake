include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

if(RunCMake_GENERATOR STREQUAL "NMake Makefiles JOM")
  # JOM removes the '$' and content following it.
else()
  test_environment_variable("TEST_DOLLAR" "x\\$x")
endif()
test_environment_variable("TEST_GENERATOR" "${RunCMake_GENERATOR}")
test_environment_variable("TEST_PRESET_NAME" "xmacrosx")
test_environment_variable("TEST_SOURCE_DIR_" "x[^\n]*[/\\\\]Tests[/\\\\]RunCMake[/\\\\]CMakePresetsBuild[/\\\\]Goodx")
test_environment_variable("TEST_SOURCE_DIR_NAME" "xGoodx")
test_environment_variable("TEST_SOURCE_PARENT_DIR" "x[^\n]*[/\\\\]Tests[/\\\\]RunCMake[/\\\\]CMakePresetsBuildx")

include("${RunCMake_SOURCE_DIR}/check.cmake")
