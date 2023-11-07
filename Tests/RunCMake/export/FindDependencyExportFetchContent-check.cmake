file(READ "${RunCMake_TEST_BINARY_DIR}/my_private_targets.cmake" my_private_targets)
if(NOT "${my_private_targets}" MATCHES "find_dependency\\(HasDeps")
  string(APPEND RunCMake_TEST_FAILED "HasDeps dependency should be exported but it is not\n")
endif()

file(READ "${RunCMake_TEST_BINARY_DIR}/my_static_targets.cmake" my_static_targets)
if(NOT "${my_static_targets}" MATCHES "find_dependency\\(MyPrivate")
  string(APPEND RunCMake_TEST_FAILED "HasDeps dependency should be exported but it is not\n")
endif()

file(READ "${RunCMake_TEST_BINARY_DIR}/my_shared_targets.cmake" my_shared_targets)
if(NOT "${my_shared_targets}" MATCHES "find_dependency\\(MyPrivate")
  string(APPEND RunCMake_TEST_FAILED "MyStatic dependency should be exported but it is not\n")
endif()
