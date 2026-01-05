# The project calls include(CTest), which creates the default output
# directory at configure time.  It must still be left unused.
set(DEFAULT_OUTPUT_DIR "${RunCMake_TEST_BINARY_DIR}/Testing")

if(NOT EXISTS "${DEFAULT_OUTPUT_DIR}")
  set(RunCMake_TEST_FAILED
    "Expected include(CTest) to create ${DEFAULT_OUTPUT_DIR}.")
  return()
endif()

file(GLOB_RECURSE DEFAULT_OUTPUT_FILES "${DEFAULT_OUTPUT_DIR}/*")
if(DEFAULT_OUTPUT_FILES)
  string(REPLACE ";" "\n  " DEFAULT_OUTPUT_FILES "${DEFAULT_OUTPUT_FILES}")
  string(APPEND RunCMake_TEST_FAILED
    "Default output directory should not have been used:\n"
    "  ${DEFAULT_OUTPUT_DIR}\n"
    "It contains:\n  ${DEFAULT_OUTPUT_FILES}\n")
  return()
endif()

if(NOT EXISTS "${OUTPUT_DIR}")
  set(RunCMake_TEST_FAILED
    "Expected output directory ${OUTPUT_DIR} does not exist.")
elseif(NOT EXISTS "${OUTPUT_DIR}/Temporary/LastTest.log")
  set(RunCMake_TEST_FAILED
    "Expected ${OUTPUT_DIR}/Temporary/LastTest.log to exist.")
endif()
