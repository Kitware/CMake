enable_language(CXX)
include(GoogleTest)

enable_testing()

set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_gtest fake_gtest.cpp)

gtest_discover_tests(
  fake_gtest
  TEST_PREFIX PREP:
  TEST_FILTER basic*
  EXTRA_ARGS how now "\"brown\" cow"
  DISCOVERY_MODE POST_BUILD
)
