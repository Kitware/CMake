include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

test_environment_variable("TEST_PRESET_NAME" "build-release")
test_environment_variable("TEST_CONFIGURE_PRESET_NAME" "release")
test_environment_variable("SOME_PATH"
  "[^\n]*[/\\\\]ConfigurePresetNameUseCase[/\\\\]build[/\\\\]release[/\\\\]generated")

include("${RunCMake_SOURCE_DIR}/check.cmake")
