
if (NOT actual_stdout MATCHES "(/|-)-PREFIXGROUP\"? +\"?.*${LINK_SHARED_LIBRARY_PREFIX}base1${LINK_SHARED_LIBRARY_SUFFIX}\"? +\"?${CMAKE_LINK_LIBRARY_FLAG}other${LINK_EXTERN_LIBRARY_SUFFIX}\"? +\"?(/|-)-SUFFIXGROUP")
  set (RunCMake_TEST_FAILED "Not found expected '--PREFIXGROUP <base1> <other> --SUFFIXGROUP'.")
endif()
