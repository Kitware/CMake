project(test-CMP0200-OLD C)

cmake_policy(SET CMP0199 NEW)
cmake_policy(SET CMP0200 OLD)

include(CMP0200-cases.cmake)

do_match_config_test(EXPECT_DEBUG EXPECT_RELEASE)
do_first_config_test(EXPECT_DEBUG)
