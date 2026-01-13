project(test-CMP0199-NEW C)

cmake_policy(SET CMP0199 NEW)
cmake_policy(SET CMP0200 NEW)

include(CMP0199-cases.cmake)

do_mapped_config_test(NEW EXPECT_RELEASE EXPECT_TEST)
