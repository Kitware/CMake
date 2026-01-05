# This project only calls enable_testing(), so the default output
# directory should never be created at all.
set(DEFAULT_OUTPUT_DIR "${RunCMake_TEST_BINARY_DIR}/Testing")

if(EXISTS "${DEFAULT_OUTPUT_DIR}")
  string(APPEND RunCMake_TEST_FAILED
    "Default output directory should not exist:\n  ${DEFAULT_OUTPUT_DIR}\n")
elseif(NOT EXISTS "${OUTPUT_DIR}")
  set(RunCMake_TEST_FAILED
    "Expected output directory ${OUTPUT_DIR} does not exist.")
elseif(NOT EXISTS "${OUTPUT_DIR}/Temporary/LastTest.log")
  set(RunCMake_TEST_FAILED
    "Expected ${OUTPUT_DIR}/Temporary/LastTest.log to exist.")
endif()
