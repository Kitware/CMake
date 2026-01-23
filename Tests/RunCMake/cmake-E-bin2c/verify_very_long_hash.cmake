execute_process(
  COMMAND "${GENERATE_VERY_LONG}"
  COMMAND "${CMAKE_COMMAND}" -E sha256sum -
  OUTPUT_VARIABLE output
  COMMAND_ERROR_IS_FATAL ANY
  )
string(SUBSTRING "${output}" 0 64 actual_sha256sum)
if(NOT actual_sha256sum STREQUAL SHA256SUM)
  message(FATAL_ERROR "Expected sha256sum for generate_very_long output:\n  ${SHA256SUM}\nActual sha256sum:\n  ${actual_sha256sum}")
endif()
