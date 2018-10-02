
include ("${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/PIE_config.cmake")

check_executable ("${pie_on}" status)
if (NOT status STREQUAL "PIE")
  set (RunCMake_TEST_FAILED "Executable is NOT 'PIE' (${status}).")
endif()
