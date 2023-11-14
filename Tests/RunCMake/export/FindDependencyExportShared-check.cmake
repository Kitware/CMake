file(READ "${RunCMake_TEST_BINARY_DIR}/mytargets.cmake" mytargets)
if("${mytargets}" MATCHES "find_dependency")
  string(APPEND RunCMake_TEST_FAILED "No dependencies should not be exported\n")
endif()
