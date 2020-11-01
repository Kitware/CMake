read_testing_file("Test.xml" _test_contents)
if(NOT _test_contents MATCHES "#CTEST_RESOURCE_GROUP_COUNT=")
  string(APPEND RunCMake_TEST_FAILED "Could not find unset variable CTEST_RESOURCE_GROUP_COUNT in test measurements\n")
endif()
