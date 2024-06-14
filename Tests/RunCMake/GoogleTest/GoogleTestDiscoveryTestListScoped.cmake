enable_language(CXX)

# 'GoogleTest' module is NOT included here by design to validate that including
# it in a subdirectory will still result in test discovery working correctly if
# 'gtest_discover_tests()' is invoked from a different scope.

enable_testing()

include(xcode_sign_adhoc.cmake)

add_subdirectory(GoogleTestDiscoveryTestListScoped)

add_gtest_executable(test_list_scoped_test test_list_test.cpp)
