file(READ "${RunCMake_TEST_BINARY_DIR}/FILTER-generated.txt" content)

set(expected "DO_NOT_FILTER_THIS;thisisanitem")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
