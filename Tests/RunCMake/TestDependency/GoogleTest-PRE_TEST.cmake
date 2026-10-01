cmake_minimum_required(VERSION 4.3)
project(TestDependencyGoogleTestPreTest CXX)

include(GoogleTest)

enable_testing()
set(CMAKE_TEST_BUILD_DEPENDS ON)

add_executable(fake_gtest fake_gtest_post_build.cpp)

gtest_discover_tests(
  fake_gtest
  TEST_PREFIX PRE:
  TEST_FILTER basic*
  DISCOVERY_MODE PRE_TEST
)
