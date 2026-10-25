file(READ "${RunCMake_TEST_BINARY_DIR}/generated.h" actual)
if (NOT actual STREQUAL "stale")
  set(RunCMake_TEST_FAILED "generated.h re-generated even though it is up to date:\n expected: stale\n actual: ${actual}")
endif()
