file(READ "${RunCMake_TEST_BINARY_DIR}/CMP0085-OLD-generated.txt" content)

set(expected "000011")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
