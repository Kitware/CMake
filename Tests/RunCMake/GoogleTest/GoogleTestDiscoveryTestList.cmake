enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(test_list_test test_list_test.cpp)
xcode_sign_adhoc(test_list_test)
gtest_discover_tests(
  test_list_test
)
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/GoogleTest-discovery-check-test-list.cmake)
