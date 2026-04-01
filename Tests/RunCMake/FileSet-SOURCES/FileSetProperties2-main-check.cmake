
if (NOT actual_stdout MATCHES "(/|-)D *OPT_INTERFACE_FS" OR actual_stdout MATCHES "(/|-)D *OPT_FS")
  set (RunCMake_TEST_FAILED "Wrong compile options specified.")
endif()
