if(NOT EXISTS ${outfile})
  set(RunCMake_TEST_FAILED "removed non-directory ${outfile}")
endif()
