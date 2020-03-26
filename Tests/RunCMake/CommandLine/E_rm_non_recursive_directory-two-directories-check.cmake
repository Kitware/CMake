if(NOT EXISTS ${out}/d1 OR NOT EXISTS ${out}/d2)
  set(RunCMake_TEST_FAILED "${out}/d1 or ${out}/d2 is removed but should not")
endif()
