# The mode set on the setup test governs the whole fixture, so the default
# CMP0224 records for the cleanup test must not override or conflict with it.
cmake_policy(SET CMP0224 NEW)
set(FIXTURE_REPEAT_MODE AROUND_ALL_REPEATS)
set(FIXTURE_REPEAT_TESTS fixture_setup)
include(${CMAKE_CURRENT_LIST_DIR}/repeat-fixture-common.cmake)
