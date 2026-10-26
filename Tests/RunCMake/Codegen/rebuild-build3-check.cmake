file(READ "${RunCMake_TEST_BINARY_DIR}/generated.h" actual)
if (NOT actual STREQUAL "2")
  set(RunCMake_TEST_FAILED "generated.h NOT re-generated after its input changed:\n expected: 2\n actual: ${actual}")
endif()
