project(test-CMP0199-NEW C)

cmake_policy(SET CMP0199 NEW)

include(CMP0199-cases.cmake)

# FIXME: CMake currently incorrectly selects the RELEASE configuration. See
# https://gitlab.kitware.com/cmake/cmake/-/issues/27022.
do_mapped_config_test(EXPECT_RELEASE)
do_unique_config_test(EXPECT_DEBUG)
