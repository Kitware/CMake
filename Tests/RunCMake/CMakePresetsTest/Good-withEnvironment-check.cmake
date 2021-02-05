include("${RunCMake_SOURCE_DIR}/../CMakePresetsBuild/TestVariable.cmake")

test_environment_variable("TEST_ENV" "Environment variable")
test_environment_variable("TEST_ENV_OVERRIDE" "Override")
test_environment_variable("TEST_ENV_OVERRIDE_REF" "xOverridex")
test_environment_variable("TEST_ENV_REF" "xEnvironment variablex")

include("${RunCMake_SOURCE_DIR}/check.cmake")
