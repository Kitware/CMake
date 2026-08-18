project(TestDependencyDiscoverCommands C)
enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(command_genex EXCLUDE_FROM_ALL fake_discovery.c)
add_executable(command_wrapper EXCLUDE_FROM_ALL fake_discovery.c)
add_executable(command_name EXCLUDE_FROM_ALL fake_discovery.c)

discover_tests(COMMAND "$<TARGET_FILE:command_genex>"
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "GENEX.\\1"
  TEST_ARGS "\\1")
discover_tests(COMMAND "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_SOURCE_DIR}/shared/fake_discovery_wrapper.cmake"
    "$<TARGET_FILE:command_wrapper>"
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "WRAPPER.\\1"
  TEST_ARGS "\\1")
discover_tests(COMMAND command_forward
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "FORWARD.\\1"
  TEST_ARGS "\\1")
discover_tests(COMMAND "$<1:command_name>"
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "NAME.\\1"
  TEST_ARGS "\\1")

add_executable(command_forward EXCLUDE_FROM_ALL fake_discovery.c)
