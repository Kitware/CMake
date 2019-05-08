file(GLOB update_xml_file "${RunCMake_TEST_BINARY_DIR}/Testing/*/Update.xml")
if(update_xml_file)
  file(READ "${update_xml_file}" update_xml LIMIT 4096)
  if(NOT update_xml MATCHES "qwertyuiop")
    string(REPLACE "\n" "\n  " update_xml "  ${update_xml}")
    set(RunCMake_TEST_FAILED
      "Did not find 'qwertyuiop' in Update.xml:\n${update_xml}"
      )
  endif()
else()
  set(RunCMake_TEST_FAILED "Update.xml not found")
endif()
