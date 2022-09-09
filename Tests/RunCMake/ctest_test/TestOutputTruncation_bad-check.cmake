file(GLOB test_xml_file "${RunCMake_TEST_BINARY_DIR}/Testing/*/Test.xml")
if(test_xml_file)
  set(RunCMake_TEST_FAILED "Test.xml should not exist:\n ${test_xml_file}")
endif()
