add_executable(example IMPORTED)
set_property(TARGET example PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/example)
include(GoogleTest)
gtest_discover_tests(example DISCOVERY_MODE PRE_TEST
  PROPERTIES
    PROP1 VAL1
    PROP2 # no VAL2
)
