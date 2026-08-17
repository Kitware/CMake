# Tests created by discovery cannot be reached by policy CMP0224 when it is
# read, so discover_tests() records the mode the policy chose for them.
cmake_policy(SET CMP0224 NEW)
enable_language(C)
enable_testing()

add_executable(fake_discovery fake_discovery.c)

discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^(case_one),LBL1$"
  TEST_NAME "setup_\\1"
  TEST_ARGS "\\1"
  TEST_PROPERTIES
    FIXTURES_SETUP DiscoveredFixture
)

add_test(NAME needs_fixture COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(needs_fixture PROPERTIES
  FIXTURES_REQUIRED DiscoveredFixture)
