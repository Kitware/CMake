file(GLOB upload_xml_file "${RunCMake_TEST_BINARY_DIR}/Testing/*/Upload.xml")
if(upload_xml_file)
  file(READ "${upload_xml_file}" upload_xml LIMIT 4096)
  if(NOT upload_xml MATCHES "<Time>")
    string(REPLACE "\n" "\n  " upload_xml "  ${upload_xml}")
    set(RunCMake_TEST_FAILED
      "Upload.xml does not contain a <Time> attribute:\n${upload_xml}"
      )
  endif()
else()
  set(RunCMake_TEST_FAILED "Upload.xml not found")
endif()
