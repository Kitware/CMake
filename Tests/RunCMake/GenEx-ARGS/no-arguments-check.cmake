file(READ "${RunCMake_TEST_BINARY_DIR}/no-arguments-generated.txt" content)

if(NOT content MATCHES "><|><")
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[><|><]]")
endif()
