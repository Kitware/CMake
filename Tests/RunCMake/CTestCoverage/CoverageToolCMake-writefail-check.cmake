set(tree "${RunCMake_TEST_BINARY_DIR}")
if(NOT IS_DIRECTORY "${tree}/wf_cmake-coverage")
  string(APPEND RunCMake_TEST_FAILED "Missing ${tree}/wf_cmake-coverage\n")
endif()
