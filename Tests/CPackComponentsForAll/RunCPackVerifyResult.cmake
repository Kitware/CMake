message(STATUS "=============================================================================")
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
message(STATUS "")

if(NOT CPackComponentsForAll_BINARY_DIR)
  message(FATAL_ERROR "CPackComponentsForAll_BINARY_DIR not set")
endif(NOT CPackComponentsForAll_BINARY_DIR)

if(NOT CPackGen)
  message(FATAL_ERROR "CPackGen not set")
endif(NOT CPackGen)

if(NOT CPackCommand)
  message(FATAL_ERROR "CPackCommand not set")
endif(NOT CPackCommand)

if(NOT CPackComponentWay)
  message(FATAL_ERROR "CPackComponentWay not set")
endif(NOT CPackComponentWay)

set(expected_file_mask "")
# The usual default behavior is to expect a single file
# Then some specific generators (Archive, RPM, ...)
# May produce several numbers of files depending on
# CPACK_COMPONENT_xxx values
set(expected_count 1)

execute_process(COMMAND ${CPackCommand} -G ${CPackGen}
    RESULT_VARIABLE CPack_result
    OUTPUT_VARIABLE CPack_output
    ERROR_VARIABLE CPack_error
    WORKING_DIRECTORY ${CPackComponentsForAll_BINARY_DIR})

if (CPack_result)
  message(SEND_ERROR "CPack_output=${CPack_output}")
  message(SEND_ERROR "CPack_error=${CPack_error}")
  message(FATAL_ERROR "error: CPack execution went wrong!")
endif(CPack_result)

if(CPackGen MATCHES "ZIP")
    set(expected_file_mask "${CPackComponentsForAll_BINARY_DIR}/MyLib-*.zip")
    if (${CPackComponentWay} STREQUAL "default")
        set(expected_count 1)
    endif(${CPackComponentWay} STREQUAL "default")
endif(CPackGen MATCHES "ZIP")

# Now verify if the number of expected file is OK
# - using expected_file_mask and
# - expected_count
if(expected_file_mask)
  file(GLOB expected_file "${expected_file_mask}")

  message(STATUS "expected_count='${expected_count}'")
  message(STATUS "expected_file='${expected_file}'")
  message(STATUS "expected_file_mask='${expected_file_mask}'")

  if(NOT expected_file)
    message(SEND_ERROR "CPack_output=${CPack_output}")
    message(SEND_ERROR "CPack_error=${CPack_error}")
    message(FATAL_ERROR "error: expected_file=${expected_file} does not exist: CPackComponentsForAll test fails.")
  endif(NOT expected_file)

  list(LENGTH expected_file actual_count)
  message(STATUS "actual_count='${actual_count}'")
  if(NOT actual_count EQUAL expected_count)
    message(SEND_ERROR "CPack_output=${CPack_output}")
    message(SEND_ERROR "CPack_error=${CPack_error}")
    message(FATAL_ERROR "error: expected_count=${expected_count} does not match actual_count=${actual_count}: CPackComponents test fails.")
  endif(NOT actual_count EQUAL expected_count)
endif(expected_file_mask)
