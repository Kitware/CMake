
if (NOT actual_stdout MATCHES "BADFLAG_\\$dollar")
  set (RunCMake_TEST_FAILED "Not found expected 'BADFLAG_$dollar'.")
endif()
