cmake_minimum_required(VERSION 3.10)
# Verify that CTEST_* variables defined via -D on the ctest command line
# are available in the script.
message("CTEST_BUILD_NAME=${CTEST_BUILD_NAME}")
message("CTEST_SITE=${CTEST_SITE}")
message("CTEST_BUILD_FLAGS=${CTEST_BUILD_FLAGS}")
message("CTEST_BUILD_TARGET=${CTEST_BUILD_TARGET}")
message("CTEST_CMAKE_GENERATOR=${CTEST_CMAKE_GENERATOR}")
message("CTEST_EXTRA_COVERAGE_GLOB=${CTEST_EXTRA_COVERAGE_GLOB}")
message("CTEST_TIME_LIMIT=${CTEST_TIME_LIMIT}")
