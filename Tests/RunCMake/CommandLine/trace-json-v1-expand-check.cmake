if(PYTHON_EXECUTABLE)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "${RunCMake_SOURCE_DIR}/trace-json-v1-check.py" --expand "${RunCMake_BINARY_DIR}/json-v1-expand.trace"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    )
  if(NOT result EQUAL 0)
    set(RunCMake_TEST_FAILED "JSON trace validation failed:\n${output}")
  endif()
endif()
