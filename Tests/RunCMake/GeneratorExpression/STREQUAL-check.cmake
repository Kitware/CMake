file(READ "${RunCMake_TEST_BINARY_DIR}/STREQUAL-generated.txt" content)

set(expected "1:0")
if(NOT content STREQUAL expected)
  set(RunCMake_TEST_FAILED "$<STREQUAL>: actual content:\n [[${content}]]\nbut expected:\n [[${expected}]]")
endif()
