file(READ "${RunCMake_TEST_BINARY_DIR}/ll.txt" actual)
file(READ "${RunCMake_TEST_BINARY_DIR}/expected.txt" expected)
string(STRIP "${actual}" actual)
string(STRIP "${expected}" expected)
# APPLY over app's LINK_LIBRARIES must equal the explicit per-library
# $<TARGET_FILE_NAME> expansion exactly.
if(NOT actual STREQUAL expected)
  set(RunCMake_TEST_FAILED
    "APPLY-over-LINK_LIBRARIES output does not match the explicit expansion:\n"
    "  actual:   [${actual}]\n"
    "  expected: [${expected}]")
endif()
