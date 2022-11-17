check_cpack_packages("TGZ;TXZ" [[TEST_ENV not defined
TEST_ENV_REF=xx
TEST_ENV_OVERRIDE not defined
TEST_ENV_OVERRIDE_REF not defined
]])

include("${RunCMake_SOURCE_DIR}/check.cmake")
