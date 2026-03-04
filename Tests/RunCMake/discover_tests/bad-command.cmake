enable_language(C)
enable_testing()

add_executable(fake_discovery fake_discovery.c)

discover_tests(COMMAND fake_discovery_notfound
  DISCOVERY_ARGS --list
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "DT.\\1"
  TEST_ARGS --run "\\1"
)
