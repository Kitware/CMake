include("${CMAKE_CURRENT_LIST_DIR}/../CXXModules/check-json.cmake")

# Check that the SARIF results from a test match the expected results
macro(check_sarif_output sarif_output_file expected_sarif_output_file)
  # Make sure the output file exists before reading it
  if (NOT EXISTS "${sarif_output_file}")
    message(FATAL_ERROR "SARIF output file not found: ${sarif_output_file}")
  endif()
  file(READ "${sarif_output_file}" actual_output)

  # The tool invocation data is specific to one run of CMake and should not be
  # checked against the fixture. Extract it and perform some dynamic checks.
  string(JSON sarif_tool_invocation GET "${actual_output}" runs 0 invocations 0)
  string(JSON actual_output REMOVE "${actual_output}" runs 0 invocations)

  # Make sure the expected output file exists before reading it
  if (NOT EXISTS "${expected_sarif_output_file}")
    message(FATAL_ERROR "Expected SARIF output file not found: ${expected_sarif_output_file}")
  endif()
  file(READ "${expected_sarif_output_file}" expected_output)

  # Check the actual output against the expected output
  check_json("${actual_output}" "${expected_output}")

  # Check the claimed path to the CMake executable used in the test.
  string(JSON sarif_invocation_exe GET "${sarif_tool_invocation}" executableLocation uri)
  if (NOT sarif_invocation_exe STREQUAL "${CMAKE_COMMAND}")
    string(APPEND RunCMake_TEST_FAILED
      "Tool executable path in SARIF does not match actual CMake driver:"
      "\n ${sarif_invocation_exe}\n expected:\n ${CMAKE_COMMAND}")
  endif()

  # Check that the timestamps are somewhat reasonable. Do not allow the epoch,
  # and make sure the end time is not before the start time.
  string(JSON sarif_invocation_start GET "${sarif_tool_invocation}" startTimeUtc)
  string(JSON sarif_invocation_end GET "${sarif_tool_invocation}" endTimeUtc)
  if (sarif_invocation_start MATCHES "^1970-01-01T00:00:00Z?$")
    string(APPEND RunCMake_TEST_FAILED
      "SARIF startTimeUtc for CMake invocation looks like the epoch:\n ${sarif_invocation_start}\n")
  endif()
  if (sarif_invocation_end MATCHES "^1970-01-01T00:00:00Z?$")
    string(APPEND RunCMake_TEST_FAILED
      "SARIF endTimeUtc for CMake invocation looks like the epoch:\n ${sarif_invocation_end}\n")
  endif()
  if (sarif_invocation_end STRLESS sarif_invocation_start)
    string(APPEND RunCMake_TEST_FAILED
      "SARIF invocation endTimeUtc is before startTimeUtc:\n"
      " startTimeUtc:\n ${sarif_invocation_start}\n endTimeUtc:\n  ${sarif_invocation_end}\n")
  endif()

  # Check that the reported exit code matches the expectation for this test.
  string(JSON sarif_invocation_exit_code GET "${sarif_tool_invocation}" exitCode)
  if (NOT sarif_invocation_exit_code EQUAL "${expect_result}")
    string(APPEND RunCMake_TEST_FAILED
      "SARIF invocation exitCode does not match:\n"
      " expected:\n ${expect_result}\n actual:\n ${sarif_invocation_exit_code}\n")
  endif()

  string(JSON sarif_invocation_success GET "${sarif_tool_invocation}" executionSuccessful)
  set(sarif_invocation_expected_success "ON")
  if (NOT "${expect_result}" EQUAL 0)
    set(sarif_invocation_expected_success "OFF")
  endif()
  if (NOT sarif_invocation_success STREQUAL sarif_invocation_expected_success)
    string(APPEND RunCMake_TEST_FAILED
      "SARIF invocation executionSuccessful does not match:\n"
      " expected:\n ${sarif_invocation_expected_success}\n actual:\n ${sarif_invocation_success}\n")
  endif()
endmacro()
