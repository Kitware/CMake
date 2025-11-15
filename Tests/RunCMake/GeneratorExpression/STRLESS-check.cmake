file(READ "${RunCMake_TEST_BINARY_DIR}/STRLESS-generated.txt" content)

set(expected "1:1:0:0")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "$<STRLESS>: actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
