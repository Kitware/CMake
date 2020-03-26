
if (actual_stdout MATCHES "BADFLAG_INTERFACE")
  string (APPEND RunCMake_TEST_FAILED "\nFound unexpected 'BADFLAG_INTERFACE'.")
endif()
