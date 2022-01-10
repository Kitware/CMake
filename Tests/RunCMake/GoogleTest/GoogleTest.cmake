enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(fake_gtest fake_gtest.cpp)
xcode_sign_adhoc(fake_gtest)

gtest_discover_tests(
  fake_gtest
  TEST_PREFIX TEST:
  TEST_SUFFIX !1
  EXTRA_ARGS how now "\"brown\" cow"
  PROPERTIES LABELS TEST1
)

gtest_discover_tests(
  fake_gtest
  TEST_PREFIX TEST:
  TEST_SUFFIX !2
  EXTRA_ARGS how now "\"brown\" cow"
  PROPERTIES LABELS TEST2
)

gtest_discover_tests(
  fake_gtest
  TEST_PREFIX TEST:
  TEST_SUFFIX !3
  TEST_FILTER basic*
  EXTRA_ARGS how now "\"brown\" cow"
  PROPERTIES LABELS TEST3
)

gtest_discover_tests(
  fake_gtest
  TEST_PREFIX TEST:
  TEST_SUFFIX !4
  TEST_FILTER typed*
  EXTRA_ARGS how now "\"brown\" cow"
  PROPERTIES LABELS TEST4
)

add_executable(no_tests_defined no_tests_defined.cpp)
xcode_sign_adhoc(no_tests_defined)

gtest_discover_tests(
  no_tests_defined
)

# Note change in behavior of TIMEOUT keyword in 3.10.3
# where it was renamed to DISCOVERY_TIMEOUT to prevent it
# from shadowing the TIMEOUT test property. Verify the
# 3.10.3 and later behavior, old behavior added in 3.10.1
# is not supported.
add_executable(property_timeout_test timeout_test.cpp)
xcode_sign_adhoc(property_timeout_test)
target_compile_definitions(property_timeout_test PRIVATE sleepSec=10)

gtest_discover_tests(
  property_timeout_test
  TEST_PREFIX property_
  TEST_SUFFIX _no_discovery
  PROPERTIES TIMEOUT 2
)
gtest_discover_tests(
  property_timeout_test
  TEST_PREFIX property_
  TEST_SUFFIX _with_discovery
  DISCOVERY_TIMEOUT 20
  PROPERTIES TIMEOUT 2
)

add_executable(skip_test skip_test.cpp)
xcode_sign_adhoc(skip_test)

gtest_discover_tests(
  skip_test
)
