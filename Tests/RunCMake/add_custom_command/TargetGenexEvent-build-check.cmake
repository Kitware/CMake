if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/wdir/touched")
  set(RunCMake_TEST_FAILED "File not created by target-dependent add_custom_command()!")
endif()
