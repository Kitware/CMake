if(EXISTS "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
  set(RunCMake_TEST_FAILED "compile_commands.json generated with CMAKE_EXPORT_COMPILE_COMMANDS overridden")
endif()
