include("${CMAKE_CURRENT_LIST_DIR}/very_long_params.cmake")
include("${CMAKE_CURRENT_BINARY_DIR}/very_long_executables.cmake")

execute_process(
  COMMAND "${generate_very_long}"
  COMMAND "${CMAKE_COMMAND}" -E bin2c --template-file "${CMAKE_CURRENT_LIST_DIR}/very_long.c.in.txt"
  COMMAND "${CMAKE_COMMAND}" -E sha256sum -
  OUTPUT_VARIABLE output
  COMMAND_ERROR_IS_FATAL ANY
  )
string(SUBSTRING "${output}" 0 64 actual_hash)
if(NOT actual_hash STREQUAL very_long_c_lf_hash AND NOT actual_hash STREQUAL very_long_c_crlf_hash)
  message(FATAL_ERROR "Expected hash of output to be ${very_long_c_lf_hash} or ${very_long_c_crlf_hash}, got ${actual_hash}")
endif()
