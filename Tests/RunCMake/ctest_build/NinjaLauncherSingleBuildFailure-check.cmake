file(GLOB build_xml_file "${RunCMake_TEST_BINARY_DIR}/Testing/*/Build.xml")
if(build_xml_file)
  file(READ "${build_xml_file}" build_xml LIMIT 8192)
  string(FIND "${build_xml}" [[This should not be compiled]] expected_failure_pos)
  if(expected_failure_pos EQUAL "-1")
    string(REPLACE "\n" "\n  " build_xml "  ${build_xml}")
    set(RunCMake_TEST_FAILED
      "Build.xml does not have expected error message:\n${build_xml}"
      )
  else()
    string(SUBSTRING "${build_xml}" "${expected_failure_pos}" -1 remaining_xml)
    string(FIND "${remaining_xml}" [[<Failure type="Error">]] unexpected_failure_pos)
    if(NOT unexpected_failure_pos EQUAL "-1")
      string(SUBSTRING "${remaining_xml}" "${unexpected_failure_pos}" -1 error_msg_xml)
      string(REPLACE "\n" "\n  " error_msg_xml "  ${error_msg_xml}")
      set(RunCMake_TEST_FAILED
        "Build.xml contains unexpected extra <Failure> elements:\n${error_msg_xml}"
        )
    endif()
  endif()
else()
  set(RunCMake_TEST_FAILED "Build.xml not found")
endif()
