# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# If changing this file, also update Utilities/Sphinx/CTestConfig.cmake

set(CTEST_PROJECT_NAME "CMake")
set(CTEST_NIGHTLY_START_TIME "1:00:00 UTC")

if(NOT CTEST_DROP_METHOD STREQUAL "https")
  set(CTEST_DROP_METHOD "http")
endif()
set(CTEST_DROP_SITE "open.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=CMake")
set(CTEST_DROP_SITE_CDASH TRUE)
set(CTEST_CDASH_VERSION "1.6")
set(CTEST_CDASH_QUERY_VERSION TRUE)
