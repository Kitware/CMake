file(READ "${RunCMake_TEST_BINARY_DIR}/CONFIG-empty-entries-generated.txt" content)

set(expected "1234")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
