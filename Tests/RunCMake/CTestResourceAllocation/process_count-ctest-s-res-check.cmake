verify_ctest_resources()

read_testing_file("Test.xml" _test_contents)
if(NOT _test_contents MATCHES "\nCTEST_RESOURCE_GROUP_0=widgets")
  string(APPEND RunCMake_TEST_FAILED "Could not find variable CTEST_RESOURCE_GROUP_0 in test measurements\n")
endif()
if(NOT _test_contents MATCHES "\nCTEST_RESOURCE_GROUP_0_WIDGETS=id:")
  string(APPEND RunCMake_TEST_FAILED "Could not find variable CTEST_RESOURCE_GROUP_0_WIDGETS in test measurements\n")
endif()
