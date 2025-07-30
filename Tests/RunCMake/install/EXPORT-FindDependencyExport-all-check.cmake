file(READ "${RunCMake_TEST_BINARY_DIR}/root-all/lib/cmake/mylib/mylib-targets.cmake" contents)

if(NOT contents MATCHES "__find_dependency_no_return\\(P2")
  string(APPEND RunCMake_TEST_FAILED "P2 dependency should be exported but it is not\n")
endif()

if(NOT contents MATCHES "__find_dependency_no_return\\(P1")
  string(APPEND RunCMake_TEST_FAILED "P1 dependency should be exported but it is not\n")
endif()

if(NOT contents MATCHES "__find_dependency_no_return\\(P2[^
]*\\)(.*)
__find_dependency_no_return\\(P1")
  string(APPEND RunCMake_TEST_FAILED "Dependencies are not in the correct order\n")
endif()
