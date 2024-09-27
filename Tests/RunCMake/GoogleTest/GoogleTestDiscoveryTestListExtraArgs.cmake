enable_language(CXX)
include(GoogleTest)

enable_testing()

include(xcode_sign_adhoc.cmake)

add_executable(test_list_extra_args test_list_extra_args.cpp)
xcode_sign_adhoc(test_list_extra_args)
gtest_discover_tests(
    test_list_extra_args
    DISCOVERY_EXTRA_ARGS "how now" "" "\"brown\" cow"
)
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/GoogleTest-discovery-check-test-list-extra-args.cmake)
