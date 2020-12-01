if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/iface.txt")
  set(RunCMake_TEST_FAILED "iface target not built")
  return()
endif()
