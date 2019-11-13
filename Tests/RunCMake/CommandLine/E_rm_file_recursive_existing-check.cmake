if(EXISTS ${out}/existing.txt)
  set(RunCMake_TEST_FAILED "${out}/existing.txt not removed")
endif()
