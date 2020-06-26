project(test_include_dirs LANGUAGES CXX)
include(GoogleTest)

enable_testing()

add_executable(discovery_timeout_test timeout_test.cpp)
target_compile_definitions(discovery_timeout_test PRIVATE discoverySleepSec=10)
gtest_discover_tests(
  discovery_timeout_test
  TEST_PREFIX discovery_
  DISCOVERY_TIMEOUT 2
  DISCOVERY_MODE ${DISCOVERY_MODE}
)
