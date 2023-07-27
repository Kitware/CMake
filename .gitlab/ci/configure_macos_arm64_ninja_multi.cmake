if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_ISPC "ON" CACHE STRING "")
  set(CMAKE_TESTS_CDASH_SERVER "https://open.cdash.org" CACHE STRING "")
endif()

# FIXME: sccache sometimes fails with "Compiler killed by signal 9".
# This job does not compile much anyway, so suppress it for now.
set(configure_no_sccache 1)

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
