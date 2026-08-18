project(TestDependencyDiscoverUtility C)
enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_discovery EXCLUDE_FROM_ALL fake_discovery.c)
add_custom_target(discovery_utility
  COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/utility-built.txt"
  VERBATIM)

discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "UTILITY.\\1"
  TEST_ARGS "\\1"
  BUILD_DEPENDS discovery_utility)
add_test(NAME UtilityBuilt
  COMMAND "${CMAKE_COMMAND}" -E compare_files
    "${CMAKE_CURRENT_BINARY_DIR}/utility-built.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/utility-built.txt")
