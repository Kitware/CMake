file(GLOB build_xml_file "${RunCMake_TEST_BINARY_DIR}/Testing/*/Build.xml")
if(build_xml_file)
  file(STRINGS "${build_xml_file}" build_cmd LIMIT_COUNT 1 REGEX "<BuildCommand>")
  if(NOT build_cmd MATCHES [[ --parallel "1"]])
    set(RunCMake_TEST_FAILED
      "Build.xml does not have expected build command with --parallel flag"
      )
  endif()
else()
  set(RunCMake_TEST_FAILED "Build.xml not found")
endif()
