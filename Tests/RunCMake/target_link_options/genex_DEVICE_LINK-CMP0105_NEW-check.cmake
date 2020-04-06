
if (NOT actual_stdout MATCHES "BADFLAG_DEVICE_LINK")
  set (RunCMake_TEST_FAILED "Not found expected 'BADFLAG_DEVICE_LINK'.")
endif()
