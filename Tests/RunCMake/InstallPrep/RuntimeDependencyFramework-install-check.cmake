check_installed([[^$]])

execute_process(COMMAND "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/runtime_dependency_framework_root"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  set(RunCMake_TEST_FAILED "Prepared framework dependency root failed: ${result}")
endif()
