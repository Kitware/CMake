if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply)
  set(RunCMake_TEST_FAILED "Failed to read FileAPI query from user config directory")
endif()
