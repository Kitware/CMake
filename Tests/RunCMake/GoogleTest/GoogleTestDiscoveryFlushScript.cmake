enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(flush_script_test flush_script_test.cpp)
xcode_sign_adhoc(flush_script_test)
gtest_discover_tests(
  flush_script_test
)

configure_file(GoogleTest-discovery-flush-script-check-list.cmake.in
  check-test-lists.cmake
  @ONLY
)
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/check-test-lists.cmake
)
