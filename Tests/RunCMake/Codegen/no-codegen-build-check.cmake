# Verify generated.hpp was NOT created
set(unexpected "${RunCMake_TEST_BINARY_DIR}/generated.hpp")
if(EXISTS "${unexpected}")
  set(RunCMake_TEST_FAILED "unexpected file created:\n  ${unexpected}")
endif()
