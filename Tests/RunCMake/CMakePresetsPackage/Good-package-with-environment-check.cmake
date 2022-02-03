check_cpack_packages("TGZ;TXZ" [[TEST_ENV=Environment variable
TEST_ENV_REF=xEnvironment variablex
TEST_ENV_OVERRIDE=Override
TEST_ENV_OVERRIDE_REF=xOverridex
]])

include("${RunCMake_SOURCE_DIR}/check.cmake")
