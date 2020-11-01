if(EXISTS ${out}/d1 OR EXISTS ${out}/d2)
  set(RunCMake_TEST_FAILED "${out}/d1 or ${out}/d2 should be removed")
endif()
