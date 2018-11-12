
include ("${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/PIE_config.cmake")

check_executable ("${pie_off}" status)
if (NOT status STREQUAL "NO_PIE")
  set (RunCMake_TEST_FAILED "Executable is NOT 'no PIE' (${status}).")
endif()
