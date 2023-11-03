cmake_minimum_required(VERSION 3.8)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

# Pick up from where the configure left off.
ctest_start(APPEND)

include(ProcessorCount)
ProcessorCount(nproc)
if (NOT "$ENV{CTEST_MAX_PARALLELISM}" STREQUAL "")
  if (nproc GREATER "$ENV{CTEST_MAX_PARALLELISM}")
    set(nproc "$ENV{CTEST_MAX_PARALLELISM}")
  endif ()
endif ()

set(CTEST_MEMORYCHECK_TYPE "$ENV{CTEST_MEMORYCHECK_TYPE}")
set(CTEST_MEMORYCHECK_SANITIZER_OPTIONS "$ENV{CTEST_MEMORYCHECK_SANITIZER_OPTIONS}")

set(lsan_suppressions "${CMAKE_CURRENT_LIST_DIR}/ctest_memcheck_$ENV{CMAKE_CONFIGURATION}.lsan.supp")
if (EXISTS "${lsan_suppressions}")
  set(ENV{LSAN_OPTIONS} "suppressions='${lsan_suppressions}'")
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/ctest_exclusions.cmake")
ctest_memcheck(
  PARALLEL_LEVEL "${nproc}"
  TEST_LOAD "${nproc}"
  RETURN_VALUE test_result
  EXCLUDE "${test_exclusions}"
  DEFECT_COUNT defects)
ctest_submit(PARTS Test)
ctest_submit(PARTS Memcheck)

include("${CMAKE_CURRENT_LIST_DIR}/ctest_annotation.cmake")
ctest_annotation_report("${CTEST_BINARY_DIRECTORY}/annotations.json"
  "Build Summary"     "https://open.cdash.org/build/${build_id}"
  "All Tests"         "https://open.cdash.org/viewTest.php?buildid=${build_id}"
  "Dynamic Analysis"  "https://open.cdash.org/viewDynamicAnalysis.php?buildid=${build_id}"
  "Test Failures"     "https://open.cdash.org/viewTest.php?onlyfailed&buildid=${build_id}"
  "Tests Not Run"     "https://open.cdash.org/viewTest.php?onlynotrun&buildid=${build_id}"
  "Test Passes"       "https://open.cdash.org/viewTest.php?onlypassed&buildid=${build_id}")

if (test_result)
  message(FATAL_ERROR
    "Failed to test")
endif ()

if (defects)
  message(FATAL_ERROR
    "Found ${defects} memcheck defects")
endif ()
