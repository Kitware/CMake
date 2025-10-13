file(READ "${RunCMake_TEST_BINARY_DIR}/STRGREATER_EQUAL-generated.txt" content)

set(expected "0:0:1:1")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "$<STRGREATER_EQUAL>: actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
