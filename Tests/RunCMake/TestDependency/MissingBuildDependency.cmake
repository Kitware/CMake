enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_test(NAME Direct
  COMMAND "${CMAKE_COMMAND}" -E true
  BUILD_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dependency.txt")
discover_tests(
  COMMAND "${CMAKE_COMMAND}" -E echo
  DISCOVERY_ARGS Discovered
  DISCOVERY_MATCH "Discovered"
  TEST_NAME Discovered
  TEST_ARGS Discovered
  BUILD_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dependency.txt")
