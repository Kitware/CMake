include("${RunCMake_SOURCE_DIR}/../CMakePresetsBuild/TestVariable.cmake")

test_environment_variable("TEST_PRESET_NAME" "test")
test_environment_variable("TEST_CONFIGURE_PRESET_NAME" "configure")

include("${RunCMake_SOURCE_DIR}/check.cmake")
