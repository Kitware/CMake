set(cache_file "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt")
if(NOT EXISTS "${cache_file}")
  set(RunCMake_TEST_FAILED
    "CMakeCache.txt not found after configure:\n  ${cache_file}")
endif()
