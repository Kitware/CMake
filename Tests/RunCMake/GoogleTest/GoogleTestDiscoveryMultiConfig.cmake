enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(configuration_gtest configuration_gtest.cpp)
xcode_sign_adhoc(configuration_gtest)
target_compile_definitions(configuration_gtest PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)

gtest_discover_tests(
  configuration_gtest
  PROPERTIES LABELS CONFIG
  DISCOVERY_MODE PRE_TEST
)
