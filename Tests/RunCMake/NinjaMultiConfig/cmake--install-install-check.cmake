if (NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/install/bin/Debug/exe${CMAKE_EXECUTABLE_SUFFIX} OR
    NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/install/bin/Release/exe${CMAKE_EXECUTABLE_SUFFIX})
  set(RunCMake_TEST_FAILED "Multi-Config Install with CMAKE_DEFAULT_CONFIGS set did not install all specified configs by default")
endif()
