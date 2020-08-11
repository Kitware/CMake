project(test_include_dirs LANGUAGES CXX)
include(GoogleTest)

enable_testing()

add_executable(configuration_gtest configuration_gtest.cpp)
target_compile_definitions(configuration_gtest PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)

gtest_discover_tests(
  configuration_gtest
  PROPERTIES LABELS CONFIG
  DISCOVERY_MODE PRE_TEST
)
