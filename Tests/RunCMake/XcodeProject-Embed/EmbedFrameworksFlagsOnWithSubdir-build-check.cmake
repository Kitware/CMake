if(NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/Debug/app.app/Contents/Frameworks/subdir/${EXTERNAL_DEPENDENCY_NAME})
  set(RunCMake_TEST_FAILED "${EXTERNAL_DEPENDENCY_NAME} was not embedded at the expected location")
endif()
