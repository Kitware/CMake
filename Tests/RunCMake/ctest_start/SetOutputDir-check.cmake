# This case never configures the project, so nothing creates the default
# output directory.  Check that the run did not create it either.
file(GLOB_RECURSE DEFAULT_OUTPUT_FILES "${DEFAULT_OUTPUT_DIR}/*")
if(DEFAULT_OUTPUT_FILES)
  string(REPLACE ";" "\n  " DEFAULT_OUTPUT_FILES "${DEFAULT_OUTPUT_FILES}")
  string(APPEND RunCMake_TEST_FAILED
    "Default output directory should not have been used:\n"
    "  ${DEFAULT_OUTPUT_DIR}\n"
    "It contains:\n  ${DEFAULT_OUTPUT_FILES}\n")
elseif(NOT EXISTS "${OUTPUT_DIR}/TAG")
  set(RunCMake_TEST_FAILED
    "Expected ${OUTPUT_DIR}/TAG file to exist.")
elseif(NOT EXISTS "${OUTPUT_DIR}/Temporary/LastStart.log")
  set(RunCMake_TEST_FAILED
    "Expected ${OUTPUT_DIR}/Temporary/LastStart.log file to exist.")
endif()
