project(test-CMP0199-NEW C)

cmake_policy(SET CMP0199 NEW)

# Note: Under CMP0199 OLD, CMake (incorrectly) selects the RELEASE
# configuration for the mapped config test. The CMP0199-NEW+CMP0200-NEW tests
# the combination of fixes.
cmake_policy(SET CMP0200 OLD)

include(CMP0199-cases.cmake)

do_mapped_config_test(EXPECT_RELEASE)
