discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  TEST_NAME "DT.\\1"
  TEST_ARGS --run "\\1"
  TEST_PROPERTIES
    LABELS UNIT AUTO
)
