
if (NOT actual_stdout MATCHES "(/|-)-START_GROUP\"? +\"?(/|-)-LIBFLAG.*${LINK_SHARED_LIBRARY_PREFIX}base1${LINK_SHARED_LIBRARY_SUFFIX}\"? +.*${LINK_SHARED_LIBRARY_PREFIX}base2${LINK_SHARED_LIBRARY_SUFFIX} +\"?(/|-)-END_GROUP")
  set (RunCMake_TEST_FAILED "Not found expected '--START_GROUP --LIBFLAG<base1> <base2> --END_GROUP'.")
endif()
