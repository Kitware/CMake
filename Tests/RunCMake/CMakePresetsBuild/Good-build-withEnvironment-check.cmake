include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

test_environment_variable("TEST_ENV_" "Environment variable")
test_environment_variable("TEST_ENV_OVERRIDE_" "Overridden")
test_environment_variable("TEST_ENV_OVERRIDE_REF" "xOverriddenx")
test_environment_variable("TEST_ENV_REF" "xEnvironment variablex")

include("${RunCMake_SOURCE_DIR}/check.cmake")
