# Verify generated.h was NOT created
set(unexpected "${RunCMake_TEST_BINARY_DIR}/generated.h")
if(EXISTS "${unexpected}")
  set(RunCMake_TEST_FAILED "unexpected file created:\n  ${unexpected}")
endif()
