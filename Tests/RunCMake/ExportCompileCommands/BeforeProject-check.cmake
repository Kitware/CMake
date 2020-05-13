if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
  set(RunCMake_TEST_FAILED "compile_commands.json not generated")
  return()
endif()
