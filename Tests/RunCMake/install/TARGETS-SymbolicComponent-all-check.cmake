file(READ "${RunCMake_TEST_BINARY_DIR}/root-all/foo.cmake" foo)

if("${foo}" MATCHES "add_library\(foo INTERFACE SYMBOLIC IMPORTED\)")
  string(APPEND RunCMake_TEST_FAILED "Symbolic Component Foo was not exported\n")
endif()
