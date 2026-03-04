enable_language(C)
enable_testing()

add_executable(fake_discovery fake_discovery.c)

discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_fail
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "DT.\\1"
  TEST_ARGS "\\1"
  TEST_PROPERTIES
    LABELS "\\2"
)
