
if (NOT actual_stdout MATCHES "BADFLAG_GLOBAL(\.[a-z]+)? +(-)?BADFLAG_PRIVATE")
  set (RunCMake_TEST_FAILED "options order is not respected.")
endif()
