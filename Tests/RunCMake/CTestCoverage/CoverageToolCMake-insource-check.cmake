set(tree "${RunCMake_TEST_BINARY_DIR}")
set(info "${tree}/insource.cmake-coverage.info")
if(EXISTS "${info}")
  file(READ "${info}" actual)
  set(expected "TN:insource
SF:${tree}/script.cmake
FNF:0
FNH:0
DA:1,1
LF:1
LH:1
end_of_record
")
  if(NOT actual STREQUAL expected)
    string(APPEND RunCMake_TEST_FAILED
      "${info} does not match.\nExpected:\n${expected}Actual:\n${actual}")
  endif()
else()
  string(APPEND RunCMake_TEST_FAILED "Missing ${info}\n")
endif()
