
if (NOT actual_stdout MATCHES "BADFLAG_INTERFACE")
  string (APPEND RunCMake_TEST_FAILED "\nNot found expected 'BADFLAG_INTERFACE'.")
endif()
