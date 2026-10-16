if(EXISTS ${outfile})
  set(RunCMake_TEST_FAILED "${outfile} was not removed")
endif()
