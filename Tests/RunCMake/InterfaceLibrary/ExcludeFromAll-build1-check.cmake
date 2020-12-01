if(EXISTS "${RunCMake_TEST_BINARY_DIR}/iface.txt")
  set(RunCMake_TEST_FAILED "iface target built as part of 'all'")
  return()
endif()
