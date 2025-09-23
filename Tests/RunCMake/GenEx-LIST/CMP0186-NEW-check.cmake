set(expected "
  000;1001;002
  x000;1001;x002
  x000;x01;x002
")

file(READ "${RunCMake_TEST_BINARY_DIR}/generated.txt" generated)

if(NOT generated STREQUAL expected)
  set(RunCMake_TEST_FAILED "generated:${generated}\nexpected:${expected}")
endif()
