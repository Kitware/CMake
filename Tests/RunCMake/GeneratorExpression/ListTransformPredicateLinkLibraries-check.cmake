file(READ "${RunCMake_TEST_BINARY_DIR}/filtered.txt" actual)
file(READ "${RunCMake_TEST_BINARY_DIR}/expected.txt" expected)
string(STRIP "${actual}" actual)
string(STRIP "${expected}" expected)
# PREDICATE over app's LINK_LIBRARIES must prefix only STATIC_LIBRARY targets
# and leave INTERFACE_LIBRARY targets unchanged.
if(NOT actual STREQUAL expected)
  set(RunCMake_TEST_FAILED
    "PREDICATE-over-LINK_LIBRARIES output does not match the expected list:\n"
    "  actual:   [${actual}]\n"
    "  expected: [${expected}]")
endif()
