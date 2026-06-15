# Check that Site and BuildName in Update.xml match what was passed via -D
# for the Configure step.
file(GLOB update_xml_files
     "${RunCMake_TEST_BINARY_DIR}/Testing/*/Update.xml")
if(NOT update_xml_files)
  set(RunCMake_TEST_FAILED
      "Update.xml not found in ${RunCMake_TEST_BINARY_DIR}/Testing/")
  return()
endif()
list(GET update_xml_files 0 update_xml)
file(READ "${update_xml}" content)
if(NOT content MATCHES "<Site>my-site</Site>")
  set(RunCMake_TEST_FAILED
      "Update.xml does not contain <Site>my-site</Site>\nActual content:\n${content}")
elseif(NOT content MATCHES "<BuildName>my-build-name</BuildName>")
  set(RunCMake_TEST_FAILED
      "Update.xml does not contain <BuildName>my-build-name</BuildName>\nActual content:\n${content}")
endif()
