message(STATUS "=============================================================")
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
message(STATUS "")

if(NOT CPackNSISGenerator_BINARY_DIR)
  message(FATAL_ERROR "CPackNSISGenerator_BINARY_DIR not set")
endif()

message(STATUS "CMAKE_COMMAND: ${CMAKE_COMMAND}")
message(STATUS "CMAKE_CPACK_COMMAND: ${CMAKE_CPACK_COMMAND}")
message(STATUS "CPackNSISGenerator_BINARY_DIR: ${CPackNSISGenerator_BINARY_DIR}")

if(config)
  set(_C_config -C ${config})
endif()

execute_process(COMMAND "${CMAKE_CPACK_COMMAND}"
                        ${_C_config}
  RESULT_VARIABLE CPack_result
  OUTPUT_VARIABLE CPack_output
  ERROR_VARIABLE CPack_error
  WORKING_DIRECTORY "${CPackNSISGenerator_BINARY_DIR}")

if(CPack_result)
  message(FATAL_ERROR "CPack execution went wrong!, CPack_output=${CPack_output}, CPack_error=${CPack_error}")
else ()
  message(STATUS "CPack_output=${CPack_output}")
endif()

set(expected_file_mask "${CPackNSISGenerator_BINARY_DIR}/_CPack_Packages/*/NSIS/*.nsi")
file(GLOB project_file "${expected_file_mask}")

message(STATUS "project_file='${project_file}'")
message(STATUS "expected_file_mask='${expected_file_mask}'")

if(NOT project_file)
  message(FATAL_ERROR "project_file does not exist.")
endif()

# should match !define MUI_HEADERIMAGE_BITMAP "${PROJECT_SOURCE_DIR}\header-image.bmp"
file(STRINGS "${project_file}" line REGEX "^!define MUI_HEADERIMAGE_BITMAP")
string(FIND "${line}" "header-image.bmp" output_index)
message(STATUS "Found the bitmap at index ${output_index}")
if("${output_index}" EQUAL "-1")
  message(FATAL_ERROR "MUI_HEADERIMAGE_BITMAP not found in the project")
endif()
