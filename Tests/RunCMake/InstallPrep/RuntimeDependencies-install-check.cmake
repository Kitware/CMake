check_installed([[^$]])

if(WIN32)
  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/runtime_dependencies_root.dll" AND
     NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/libruntime_dependencies_root.dll")
    set(RunCMake_TEST_FAILED "Preparation did not build the module to scan.")
  endif()
else()
  execute_process(COMMAND "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/runtime_dependencies_root${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    set(RunCMake_TEST_FAILED "Prepared runtime dependency root failed: ${result}")
  endif()
endif()
