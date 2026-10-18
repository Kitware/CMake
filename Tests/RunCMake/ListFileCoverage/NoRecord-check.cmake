if(EXISTS "${no_cov_dir}")
  set(RunCMake_TEST_FAILED "Coverage directory was created:\n  ${no_cov_dir}")
endif()
