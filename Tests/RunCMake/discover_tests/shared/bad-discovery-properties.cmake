discover_tests(COMMAND fake_discovery
  DISCOVERY_ARGS --list
  DISCOVERY_MATCH "^([^,]+),([^,]+)$"
  DISCOVERY_PROPERTIES
    LABELS UNIT AUTO
  TEST_NAME "DT.\\1"
  TEST_ARGS --run "\\1"
)
