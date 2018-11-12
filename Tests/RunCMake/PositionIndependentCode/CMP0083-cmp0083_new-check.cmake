
include ("${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/CMP0083_config.cmake")


# retrieve default type of executable
check_executable ("${cmp0083_ref}" ref)

if (ref STREQUAL "PIE")
  # check no_pie executable is really no position independent
  check_executable ("${cmp0083_new_no_pie}" new_no_pie)
  if (NOT new_no_pie STREQUAL "NO_PIE")
    set (RunCMake_TEST_FAILED "CMP0083(NEW) do not produce expected executable.")
  endif()
elseif (ref STREQUAL "NO_PIE")
  # check pie executable is really position independent
  check_executable ("${cmp0083_new_pie}" new_pie)
  if (NOT new_pie MATCHES "PIE")
    set (RunCMake_TEST_FAILED "CMP0083(NEW) do not produce expected executable.")
  endif()
else()
  set (RunCMake_TEST_FAILED "CMP0083(NEW) unexpected result.")
endif()
