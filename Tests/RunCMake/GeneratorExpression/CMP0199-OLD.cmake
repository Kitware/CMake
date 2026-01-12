project(test-CMP0199-OLD C)

cmake_policy(SET CMP0199 OLD)

include(CMP0199-cases.cmake)

do_mapped_config_test(EXPECT_RELEASE EXPECT_DEBUG EXPECT_TEST)
