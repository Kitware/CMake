
include ("${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/CMP0083_config.cmake")


# retrieve default type of executable
check_executable ("${cmp0083_ref}" ref)

# POSITION_INDEPENDENT_CODE must not have influence on executable
# pie and no_pie executable must have same type as reference
check_executable ("${cmp0083_old_pie}" old_pie)
if (NOT old_pie STREQUAL ref)
  set (RunCMake_TEST_FAILED "CMP0083(OLD) do not produce expected executable.")
  return()
endif()

check_executable ("${cmp0083_old_no_pie}" old_no_pie)
if (NOT old_no_pie STREQUAL ref)
  set (RunCMake_TEST_FAILED "CMP0083(OLD) do not produce expected executable.")
  return()
endif()
