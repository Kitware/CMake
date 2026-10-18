file(GLOB_RECURSE info_files "${RunCMake_TEST_BINARY_DIR}/*.info")
if(info_files)
  set(RunCMake_TEST_FAILED "Unexpected coverage files:\n  ${info_files}")
endif()
