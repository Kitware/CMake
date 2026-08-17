# The mode set on the cleanup test governs the whole fixture, so the setup
# test must not repeat on its own.
set(FIXTURE_REPEAT_MODE AROUND_ALL_REPEATS)
set(FIXTURE_REPEAT_TESTS fixture_cleanup)
include(${CMAKE_CURRENT_LIST_DIR}/repeat-fixture-common.cmake)
