
if (NOT actual_stdout MATCHES "(/|-)-PREFIXGROUP\"? +\"?(/|-)-LIBGROUP.*${LINK_SHARED_LIBRARY_PREFIX}base1${LINK_SHARED_LIBRARY_SUFFIX}\"? +\"?(/|-)-LIBGROUPother${LINK_EXTERN_LIBRARY_SUFFIX}\"? +\"?(/|-)-SUFFIXGROUP")
  set (RunCMake_TEST_FAILED "Not found expected '--PREFIXGROUP --LIBGROUP<base1> --LIBGROUP<other> --SUFFIXGROUP'.")
endif()
