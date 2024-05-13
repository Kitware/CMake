file(READ ${RunCMake_TEST_BINARY_DIR}/out.txt out)
if(NOT out STREQUAL "'' ''")
  set(RunCMake_TEST_FAILED "PROPERTY_THAT_IS_NOT_SET did not evaluate as empty:\n ${out}")
endif()
