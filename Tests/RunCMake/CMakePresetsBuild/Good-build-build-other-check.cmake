include("${RunCMake_SOURCE_DIR}/TestVariable.cmake")

test_environment_variable("TEST_ENV_" "other")

include("${RunCMake_SOURCE_DIR}/check.cmake")
