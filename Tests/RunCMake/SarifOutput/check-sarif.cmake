include("${CMAKE_CURRENT_LIST_DIR}/../CXXModules/check-json.cmake")

# Check that the SARIF results from a test match the expected results
macro(check_sarif_output sarif_output_file expected_sarif_output_file)
  # Make sure the output file exists before reading it
  if (NOT EXISTS "${sarif_output_file}")
    message(FATAL_ERROR "SARIF output file not found: ${sarif_output_file}")
  endif()
  file(READ "${sarif_output_file}" actual_output)

  # Make sure the expected output file exists before reading it
  if (NOT EXISTS "${expected_sarif_output_file}")
    message(FATAL_ERROR "Expected SARIF output file not found: ${expected_sarif_output_file}")
  endif()
  file(READ "${expected_sarif_output_file}" expected_output)

  # Check the actual output against the expected output
  check_json("${actual_output}" "${expected_output}")
endmacro()
