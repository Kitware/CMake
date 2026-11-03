include(${CMAKE_CURRENT_LIST_DIR}/check-coverage.cmake)
if(RunCMake_TEST_FAILED)
  return()
endif()

get_file_element(shared "${coverage_xml}" "Shared.cmake")
if(NOT shared)
  string(APPEND RunCMake_TEST_FAILED "No File for Shared.cmake in Coverage.xml\n")
endif()

# The build tree is inside the source tree, but its files stay out.
expect_no_file(Generated.cmake)
