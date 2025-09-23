set(expected "
  ;1001;2
  xxxx;1001;xxx2
  xxxx;xx1;xxx2
")

file(READ "${RunCMake_TEST_BINARY_DIR}/generated.txt" generated)

if(NOT generated STREQUAL expected)
  set(RunCMake_TEST_FAILED "generated:${generated}\nexpected:${expected}")
endif()
