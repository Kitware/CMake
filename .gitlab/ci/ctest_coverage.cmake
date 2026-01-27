cmake_minimum_required(VERSION 3.29)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

# Pick up from where the configure left off.
ctest_start(APPEND)

find_program(GCOV NAMES gcov)
set(CTEST_COVERAGE_COMMAND "${GCOV}")
set(CTEST_COVERAGE_EXTRA_FLAGS
  --hash-filenames
  --long-file-names)
string(REPLACE ";" " " CTEST_COVERAGE_EXTRA_FLAGS "${CTEST_COVERAGE_EXTRA_FLAGS}")
ctest_coverage(
  RETURN_VALUE coverage_result)
ctest_submit(PARTS Coverage)

include("${CMAKE_CURRENT_LIST_DIR}/ctest_annotation.cmake")
ctest_annotation_report("${CTEST_BINARY_DIRECTORY}/annotations.json"
  "Coverage Report" "https://open.cdash.org/viewCoverage.php?buildid=${build_id}"
)

if (coverage_result)
  message(FATAL_ERROR
    "Failed to gather coverage")
endif ()
