function (find_xml_file name)
  file(GLOB test_xml_file "${RunCMake_TEST_BINARY_DIR}/Testing/*/${name}.xml")
  if (NOT test_xml_file)
    message(FATAL_ERROR
      "${name}.xml not created.")
  endif ()
endfunction ()

find_xml_file(DynamicAnalysis)
find_xml_file(DynamicAnalysis-Test)
