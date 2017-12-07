project(test_include_dirs)
include(CTest)
include(GoogleTest)

enable_testing()

add_executable(fake_gtest fake_gtest.cpp)

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

add_executable(timeout_test timeout_test.cpp)

gtest_discover_tests(
  timeout_test
)
