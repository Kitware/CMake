file(READ "${RunCMake_TEST_BINARY_DIR}/root-all/lib/cmake/mylib/mylib-targets.cmake" contents)
if(NOT contents MATCHES "include\\(CMakeFindDependencyMacro\\)\nfind_dependency\\(P2\\)\nfind_dependency\\(P1\\)\n")
  set(RunCMake_TEST_FAILED "Dependencies were not properly exported")
endif()
