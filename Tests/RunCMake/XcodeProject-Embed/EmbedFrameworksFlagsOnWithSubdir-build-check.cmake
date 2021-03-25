if(NOT EXISTS ${RunCMake_TEST_BINARY_DIR}/Debug/app.app/Contents/Frameworks/subdir/sharedFrameworkExt.framework)
  set(RunCMake_TEST_FAILED "Framework was not embedded at the expected location")
endif()
