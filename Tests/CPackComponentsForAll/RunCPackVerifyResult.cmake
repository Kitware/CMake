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
set(expected_file_mask "")

execute_process(COMMAND ${CPackCommand} -G ${CPackGen}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
    WORKING_DIRECTORY ${CPackComponentsForAll_BINARY_DIR})

if(CPackGen MATCHES "ZIP")
    set(expected_file_mask "${CPackComponentsForAll_BINARY_DIR}/MyLib-*.zip")
    set(expected_count 2)
endif(CPackGen MATCHES "ZIP")


if(expected_file_mask)
  file(GLOB expected_file "${expected_file_mask}")

  message(STATUS "expected_count='${expected_count}'")
  message(STATUS "expected_file='${expected_file}'")
  message(STATUS "expected_file_mask='${expected_file_mask}'")

  if(NOT expected_file)
    message(FATAL_ERROR "error: expected_file does not exist: CPackComponentsForAll test fails.")
  endif(NOT expected_file)

  list(LENGTH expected_file actual_count)
  message(STATUS "actual_count='${actual_count}'")
  if(NOT actual_count EQUAL expected_count)
    message(FATAL_ERROR "error: expected_count does not match actual_count: CPackComponents test fails.")
  endif(NOT actual_count EQUAL expected_count)
endif(expected_file_mask)
