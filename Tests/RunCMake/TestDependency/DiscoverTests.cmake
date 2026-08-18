cmake_minimum_required(VERSION 4.3)
project(TestDependencyDiscoverGeneric C)

enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_discovery fake_discovery.c)

discover_tests(
  COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "DISCOVER.\\1"
  TEST_ARGS "\\1"
  TEST_PROPERTIES
    LABELS "\\2"
)
