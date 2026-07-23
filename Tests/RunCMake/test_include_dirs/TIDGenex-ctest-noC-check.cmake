# ctest invoked WITHOUT -C.  Unconditional includes always run; guarded
# per-config includes run only on single-config (where they collapse).
if(NOT actual_stdout MATCHES "plain_test")
  string(APPEND RunCMake_TEST_FAILED "plain_test missing without -C\n")
endif()
if(NOT actual_stdout MATCHES "indep_test")
  string(APPEND RunCMake_TEST_FAILED "indep_test missing without -C\n")
endif()
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  if(actual_stdout MATCHES "config_Debug|config_Release")
    string(APPEND RunCMake_TEST_FAILED
      "guarded config-dependent test ran without -C on multi-config\n")
  endif()
  if(actual_stdout MATCHES "dbgonly_test")
    string(APPEND RunCMake_TEST_FAILED
      "dbgonly_test ran without -C on multi-config\n")
  endif()
else()
  if(NOT actual_stdout MATCHES "config_Debug")
    string(APPEND RunCMake_TEST_FAILED
      "config_Debug missing on single-config without -C\n")
  endif()
  if(NOT actual_stdout MATCHES "dbgonly_test")
    string(APPEND RunCMake_TEST_FAILED
      "dbgonly_test missing on single-config without -C\n")
  endif()
endif()
