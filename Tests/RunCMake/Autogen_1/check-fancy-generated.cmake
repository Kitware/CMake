if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/fancy_generated.txt")
   set(RunCMake_TEST_FAILED "'${RunCMake_TEST_BINARY_DIR}/fancy_generated.txt' missing")
endif()
