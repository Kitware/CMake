# The discovered tests are created by a script that ctest runs, so check the
# generated file that carries their properties.
add_executable(example IMPORTED)
set_property(TARGET example PROPERTY IMPORTED_LOCATION
  ${CMAKE_CURRENT_BINARY_DIR}/example)
include(GoogleTest)
gtest_discover_tests(example DISCOVERY_MODE PRE_TEST
  PROPERTIES FIXTURES_SETUP ExampleFixture
)
