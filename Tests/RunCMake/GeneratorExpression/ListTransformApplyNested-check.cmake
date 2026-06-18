file(READ "${RunCMake_TEST_BINARY_DIR}/nested.txt" actual)
file(READ "${RunCMake_TEST_BINARY_DIR}/expected.txt" expected)
string(STRIP "${actual}" actual)
string(STRIP "${expected}" expected)
# The nested APPLY (outer binds each target, inner uppercases each include dir)
# must equal the explicit per-target expansion exactly.
if(NOT actual STREQUAL expected)
  set(RunCMake_TEST_FAILED
    "nested APPLY output does not match the explicit expansion:\n"
    "  actual:   [${actual}]\n"
    "  expected: [${expected}]")
endif()
