cmake_policy(SET CMP0140 NEW)

function(validate_json_schema schema_file input_files)
  if (NOT(Python_EXECUTABLE AND CMake_TEST_JSON_SCHEMA))
    return()
  endif()

  cmake_parse_arguments(ARG_VS "" "EXPECTED_RESULT" "" ${ARGN})

  if(NOT ARG_VS_EXPECTED_RESULT)
    set(ARG_VS_EXPECTED_RESULT 0)
  endif()

  set(_validate_py "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/validate_json_schema.py")
  cmake_path(SET _schema NORMALIZE "${schema_file}")
  set(_inputs)
  foreach(_input IN LISTS input_files)
    cmake_path(SET _input NORMALIZE "${_input}")
    list(APPEND _inputs "${_input}")
  endforeach()

  execute_process(
    COMMAND
      "${Python_EXECUTABLE}" "${_validate_py}" "${_schema}"
      "--input-files" ${_inputs}
      # Include "--verbose" here for local debugging as needed.
    RESULT_VARIABLE result
    ERROR_VARIABLE error
  )
  if(NOT result MATCHES "${ARG_VS_EXPECTED_RESULT}")
    string(REPLACE "\n" "\n  " error "${error}")
    string(APPEND RunCMake_TEST_FAILED "Failed to validate version JSON schema for file: ${json_file}\nOutput:\n${error}\nResult: ${result}\nExpected: ${ARG_VS_EXPECTED_RESULT}")
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()
