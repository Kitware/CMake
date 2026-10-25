file(READ "${RunCMake_TEST_BINARY_DIR}/generated.h" actual)
if (NOT actual STREQUAL "1")
  set(RunCMake_TEST_FAILED "generated.h has wrong content:\n expected: 1\n actual: ${actual}")
endif()
