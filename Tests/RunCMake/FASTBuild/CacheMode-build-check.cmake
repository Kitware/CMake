file(GLOB_RECURSE entries "${RunCMake_TEST_BINARY_DIR}/cache/*")
if(NOT entries)
  set(RunCMake_TEST_FAILED
    "cmake --build stored nothing in ${RunCMake_TEST_BINARY_DIR}/cache")
endif()
