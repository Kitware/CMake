if (actual_stdout MATCHES "(/|-)-LIBFLAG${LINK_SHARED_LIBRARY_PREFIX}base1${LINK_SHARED_LIBRARY_SUFFIX}")
  set (RunCMake_TEST_FAILED "Found unexpected '--LIBFLAG<base1>'.")
endif()
