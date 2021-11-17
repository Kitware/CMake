enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(fake_gtest fake_gtest.cpp)
xcode_sign_adhoc(fake_gtest)

gtest_discover_tests(
  fake_gtest
  TEST_FILTER "${TEST_FILTER}*"
)
