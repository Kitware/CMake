file(READ "${RunCMake_TEST_BINARY_DIR}/simple-generated.txt" content)

if(NOT content MATCHES "1")
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[1]]")
endif()
