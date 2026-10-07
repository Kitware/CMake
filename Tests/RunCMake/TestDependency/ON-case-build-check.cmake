if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${dependency}_case_dependency-built.txt")
  set(RunCMake_TEST_FAILED
    "test_prep did not build ${dependency}_case_dependency.")
endif()
