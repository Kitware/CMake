file(READ "${RunCMake_TEST_BINARY_DIR}/REMOVE_DUPLICATES-generated.txt" content)

set(expected "1;;2;3")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
