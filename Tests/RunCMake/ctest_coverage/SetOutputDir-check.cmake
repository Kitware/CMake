# ctest_configure() runs include(CTest), which creates the default output
# directory at configure time.  Check that nothing was written into it.
file(GLOB_RECURSE DEFAULT_OUTPUT_FILES "${DEFAULT_OUTPUT_DIR}/*")
if(DEFAULT_OUTPUT_FILES)
  string(REPLACE ";" "\n  " DEFAULT_OUTPUT_FILES "${DEFAULT_OUTPUT_FILES}")
  string(APPEND RunCMake_TEST_FAILED
    "Default output directory should not have been used:\n"
    "  ${DEFAULT_OUTPUT_DIR}\n"
    "It contains:\n  ${DEFAULT_OUTPUT_FILES}\n")
  return()
endif()

set(TAG_PATH "${OUTPUT_DIR}/TAG")
if(NOT EXISTS "${TAG_PATH}")
  set(RunCMake_TEST_FAILED "Expected ${TAG_PATH} file to exist.")
  return()
endif()

file(STRINGS "${TAG_PATH}" TAG_CONTENTS)
if(TAG_CONTENTS STREQUAL "")
  set(RunCMake_TEST_FAILED "${TAG_PATH} is empty.")
  return()
endif()

list(GET TAG_CONTENTS 0 TAG)
set(EXPECTED_FILES
  "${OUTPUT_DIR}/Temporary/LastStart.log"
  "${OUTPUT_DIR}/Temporary/LastConfigure_${TAG}.log"
  "${OUTPUT_DIR}/Temporary/LastBuild_${TAG}.log"
  "${OUTPUT_DIR}/${TAG}/Configure.xml"
  "${OUTPUT_DIR}/${TAG}/Build.xml"
  # Produced from the coverage data the test script leaves in the build tree.
  "${OUTPUT_DIR}/${TAG}/Coverage.xml"
  "${OUTPUT_DIR}/${TAG}/CoverageLog-0.xml"
)

foreach(EXP_FILE IN LISTS EXPECTED_FILES)
  if(NOT EXISTS "${EXP_FILE}")
    set(RunCMake_TEST_FAILED "Expected ${EXP_FILE} file to exist.")
    return()
  endif()
endforeach()
