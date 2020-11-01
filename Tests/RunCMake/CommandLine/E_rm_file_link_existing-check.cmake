if(NOT EXISTS ${out}/existing.txt)
  set(RunCMake_TEST_FAILED "${out}/existing.txt should exist (we only removed the link)")
endif()
