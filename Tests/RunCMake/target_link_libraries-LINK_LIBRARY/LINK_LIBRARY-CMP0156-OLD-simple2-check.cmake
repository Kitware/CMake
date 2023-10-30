
if (NOT actual_stdout MATCHES "(/|-)-LIBFLAG.*${LINK_SHARED_LIBRARY_PREFIX}base1")
  set (RunCMake_TEST_FAILED "Not found expected '--LIBFLAG<base1>'.")
endif()
