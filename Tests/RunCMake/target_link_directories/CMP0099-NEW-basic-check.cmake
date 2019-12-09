
if (NOT actual_stdout MATCHES "DIR_INTERFACE")
  string (APPEND RunCMake_TEST_FAILED "\nNot found expected 'DIR_INTERFACE'.")
endif()
