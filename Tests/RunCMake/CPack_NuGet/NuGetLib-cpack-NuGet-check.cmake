file(GLOB generated_nuspec "${RunCMake_TEST_BINARY_DIR}/_CPack_Packages/*/NuGet/GeneratorTest-1.2.3-*/CPack.NuGet.nuspec")
if(NOT generated_nuspec)
  set(RunCMake_TEST_FAILED "No nuspec file generated under ${RunCMake_TEST_BINARY_DIR}")
else()
  # Read in the generated nuspec file content
  file(READ "${generated_nuspec}" actual_nuspec)
  # Read in the expected file content
  file(READ "${CMAKE_CURRENT_LIST_DIR}/expected.nuspec" expected_nuspec)

  # Compare the file contents
  string(COMPARE EQUAL "${actual_nuspec}" "${expected_nuspec}" nuspec_matches)

  if(NOT nuspec_matches)
    set(RunCMake_TEST_FAILED "generated nuspec file incorrect")
    set(failure_msg "")
    # This would be nicer with a `diff` output, but it needs to be portable
    string(APPEND failure_msg "\nExpected file:\n")
    string(APPEND failure_msg "${expected_nuspec}")
    string(APPEND failure_msg "Actual file:\n")
    string(APPEND failure_msg "${actual_nuspec}")
    set(RunCMake_TEST_FAILURE_MESSAGE "${failure_msg}")
  endif()
endif()
