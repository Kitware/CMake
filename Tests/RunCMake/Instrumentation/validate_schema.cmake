function(validate_schema json_file schema_file expected_result)
  if (Python_EXECUTABLE AND CMake_TEST_JSON_SCHEMA)
    execute_process(
      COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/validate_schema.py" "${json_file}" "${schema_file}"
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE output
    )
    if(NOT result MATCHES ${expected_result})
      string(REPLACE "\n" "\n  " output "${output}")
      string(APPEND RunCMake_TEST_FAILED "Failed to validate version JSON schema for file: ${json_file}\nOutput:\n${output}\nResult: ${result}\nExpected: ${expected_result}")
    endif()
    return(PROPAGATE RunCMake_TEST_FAILED)
  endif()
endfunction()
