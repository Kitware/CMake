enable_language(C)
enable_testing()

add_executable(fake_discovery fake_discovery.c)

discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_tests
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "DT.\\1"
  TEST_ARGS "\\1"
  TEST_PROPERTIES
    LABELS "\\2"
)

discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list_env
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  DISCOVERY_PROPERTIES
    ENVIRONMENT "TEST_NAME=two"
    ENVIRONMENT_MODIFICATION "TEST_LABEL=string_append:BAR"
  TEST_NAME "Env.\\1"
  TEST_ARGS "\\1"
  TEST_PROPERTIES
    LABELS "\\2"
)

discover_tests(COMMAND fake_discovery --list_args "one;two"
  DISCOVERY_ARGS "three"
  DISCOVERY_MATCH ".*"
  TEST_NAME "ExpandLists.OFF.\\0"
  TEST_ARGS ""
)

discover_tests(COMMAND fake_discovery --list_args "one;two" COMMAND_EXPAND_LISTS
  DISCOVERY_ARGS "three"
  DISCOVERY_MATCH ".*"
  TEST_NAME "ExpandLists.ON.\\0"
  TEST_ARGS ""
)
