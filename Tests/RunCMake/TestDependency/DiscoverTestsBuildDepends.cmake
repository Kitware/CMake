cmake_minimum_required(VERSION 4.3)
project(TestDependencyDiscoverGenericBuildDepends C)

enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_discovery fake_discovery.c)

discover_tests(
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/shared/fake_discovery_wrapper.cmake
    "$<TARGET_FILE:fake_discovery>"
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "DISCOVER.\\1"
  TEST_ARGS "\\1"
  TEST_PROPERTIES
    LABELS "\\2"
  BUILD_DEPENDS
    fake_discovery
)
