include(${RunCMake_SOURCE_DIR}/cpack-check-common.cmake)

if(NOT wix_log MATCHES "light[^\n]* -sice:ICE57")
  string(APPEND RunCMake_TEST_FAILED "light was not run with -sice:ICE57.\n")
endif()
