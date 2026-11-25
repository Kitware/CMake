include(${CMAKE_CURRENT_LIST_DIR}/check-coverage.cmake)
if(RunCMake_TEST_FAILED)
  return()
endif()

get_file_element(shared "${coverage_xml}" "Shared.cmake")
if(NOT shared)
  string(APPEND RunCMake_TEST_FAILED "No File for Shared.cmake in Coverage.xml\n")
else()
  foreach(e IN ITEMS "<FunctionsTested>1</FunctionsTested>" "<FunctionsUnTested>1</FunctionsUnTested>"
      "<BranchesTested>2</BranchesTested>" "<BranchesUnTested>2</BranchesUnTested>")
    string(FIND "${shared}" "${e}" pos)
    if(pos EQUAL -1)
      string(APPEND RunCMake_TEST_FAILED "Coverage.xml has no ${e} for Shared.cmake:\n${shared}\n")
    endif()
  endforeach()
endif()

# Number is zero-based, so this is line 2, which both tests ran.
get_file_element(shared_log "${coverage_log_xml}" "Shared.cmake")
string(FIND "${shared_log}" "<Line Number=\"1\" Count=\"2\"" pos)
if(pos EQUAL -1)
  string(APPEND RunCMake_TEST_FAILED "CoverageLog-0.xml has no merged count for line 2 of Shared.cmake:\n${shared_log}\n")
endif()

# Number is zero-based here too.  Line 9 saw both outcomes over the two
# tests.  Line 5 never ran, so its "-" counts as untested.
foreach(e IN ITEMS
    "<Line Number=\"8\" Count=\"2\" BranchesTested=\"2\" BranchesUntested=\"0\""
    "<Line Number=\"4\" Count=\"0\" BranchesTested=\"0\" BranchesUntested=\"2\""
    )
  string(FIND "${shared_log}" "${e}" pos)
  if(pos EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED "CoverageLog-0.xml has no ${e} for Shared.cmake:\n${shared_log}\n")
  endif()
endforeach()

expect_no_file(CMakePrintHelpers.cmake)
expect_no_file(Generated.cmake)
