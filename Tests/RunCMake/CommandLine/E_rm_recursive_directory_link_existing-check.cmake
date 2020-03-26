if(NOT EXISTS ${out}/dir/existing.txt)
  set(RunCMake_TEST_FAILED "${out}/dir/existing.txt should exist (we only removed the link to dir folder)")
endif()
