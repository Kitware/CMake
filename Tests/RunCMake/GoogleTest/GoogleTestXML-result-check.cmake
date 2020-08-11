set(RESULT_FILE "${RunCMake_TEST_BINARY_DIR}/GoogleTestXML.Foo.xml")
if(NOT EXISTS ${RESULT_FILE})
  set(RunCMake_TEST_FAILED "Result XML file ${RESULT_FILE} was not created")
endif()
