
if (NOT actual_stdout MATCHES "LINKER:-foo,bar")
  set (RunCMake_TEST_FAILED "LINKER: prefix was expanded.")
  return()
endif()
