project(TestDependencyDiscoverGeneratedFile C)
enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_discovery EXCLUDE_FROM_ALL fake_discovery.c)
set(output "${CMAKE_CURRENT_BINARY_DIR}/generated.txt")
add_custom_command(OUTPUT "${output}"
  COMMAND "${CMAKE_COMMAND}" -E touch "${output}"
  VERBATIM)
add_custom_target(generated_owner DEPENDS "${output}")

discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "GENERATED.\\1"
  TEST_ARGS "\\1"
  BUILD_DEPENDS "${output}")
add_test(NAME GeneratedFileBuilt
  COMMAND "${CMAKE_COMMAND}" -E compare_files "${output}" "${output}")
