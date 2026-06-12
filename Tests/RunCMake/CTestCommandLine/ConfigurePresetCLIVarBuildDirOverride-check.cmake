# --build-dir should take precedence over the preset's binaryDir.
# Configure.xml must be in the explicit build dir, not in ${sourceDir}/build.
file(GLOB configure_xml_file
  "${RunCMake_TEST_BINARY_DIR}/Testing/*/Configure.xml")
if(NOT configure_xml_file)
  set(RunCMake_TEST_FAILED
      "Configure.xml not found in explicit --build-dir (${RunCMake_TEST_BINARY_DIR})")
endif()

if(IS_DIRECTORY "${RunCMake_TEST_SOURCE_DIR}/build")
  set(RunCMake_TEST_FAILED
      "Preset binaryDir (${RunCMake_TEST_SOURCE_DIR}/build) was created")
endif()
