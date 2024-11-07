file(READ ${stamp} content)
if(NOT content STREQUAL 5)
  set(RunCMake_TEST_FAILED "Expected stamp '5' but got: '${content}'")
endif()
