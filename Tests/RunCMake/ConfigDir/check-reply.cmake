if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply)
  set(RunCMake_TEST_FAILED "Failed to read FileAPI query from user config directory")
endif()
if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/.cmake/instrumentation-a37d1069-1972-4901-b9c9-f194aaf2b6e0/v1/data)
  set(RunCMake_TEST_FAILED "Failed to read Instrumentation query from user config directory")
endif()
