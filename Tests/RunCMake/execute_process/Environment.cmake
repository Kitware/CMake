execute_process(
  COMMAND ${CMAKE_COMMAND} -E environment
  ENVIRONMENT
    "CMAKE_TEST_RUNCMAKE_EXECUTE_PROCESS_ENVIRONMENT=expected_value"
  )
