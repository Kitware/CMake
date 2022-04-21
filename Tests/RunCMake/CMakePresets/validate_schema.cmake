function(validate_schema file expected_result)
  if (NOT CMakePresets_VALIDATE_SCRIPT_PATH)
    set(CMakePresets_VALIDATE_SCRIPT_PATH "${RunCMake_SOURCE_DIR}/validate_schema.py")
  endif()

  execute_process(
    COMMAND "${Python_EXECUTABLE}" "${CMakePresets_VALIDATE_SCRIPT_PATH}" "${file}"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _output
    ERROR_VARIABLE _error
    )
  if(NOT _result STREQUAL expected_result)
    string(REPLACE "\n" "\n  " _output_p "${_output}")
    string(REPLACE "\n" "\n  " _error_p "${_error}")
    string(APPEND RunCMake_TEST_FAILED "Expected result of validating ${file}: ${expected_result}\nActual result: ${_result}\nOutput:\n  ${_output_p}\nError:\n  ${_error_p}\n")
  endif()

  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction()
