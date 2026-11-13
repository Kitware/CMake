# Record the coverage of the CMake code that each test runs.
set(CTEST_TEST_COVERAGE_TOOL "CMAKE")

# Use the test environment of the Makefiles job, whose module tests run here.
include("${CMAKE_CURRENT_LIST_DIR}/env_fedora45_makefiles.cmake")
