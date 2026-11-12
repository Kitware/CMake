set(cmakecache_file "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt")
if(EXISTS "${cmakecache_file}")
  file(READ "${cmakecache_file}" cmakecache_txt)
  if(NOT cmakecache_txt MATCHES "MY_VAR_B:STRING=from-options")
     set(RunCMake_TEST_FAILED
         "CMakeCache.txt does not contain MY_VAR_B from OPTIONS")
  endif()
  if(cmakecache_txt MATCHES "MY_VAR_A")
     string(APPEND RunCMake_TEST_FAILED
       "\nCMakeCache.txt contains MY_VAR_A from CTEST_CONFIGURE_OPTIONS, "
       "which OPTIONS should have overridden")
  endif()
else()
  set(RunCMake_TEST_FAILED "CMakeCache.txt not found")
endif()
