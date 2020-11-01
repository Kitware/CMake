
if (actual_stdout MATCHES "DIR_INTERFACE")
  string (APPEND RunCMake_TEST_FAILED "\nFound unexpected 'DIR_INTERFACE'.")
endif()
