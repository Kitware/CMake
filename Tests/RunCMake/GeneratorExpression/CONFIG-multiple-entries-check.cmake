file(READ "${RunCMake_TEST_BINARY_DIR}/CONFIG-multiple-entries-generated.txt" content)

set(expected "14")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
