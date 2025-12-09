file(READ "${RunCMake_TEST_BINARY_DIR}/LIST-edgecases.txt" content)

set(expected "a;b;c:;d\na;b;c;:d")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
