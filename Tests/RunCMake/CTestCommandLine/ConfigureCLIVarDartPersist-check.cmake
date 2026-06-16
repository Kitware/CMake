set(dart_file "${RunCMake_TEST_BINARY_DIR}/DartConfiguration.tcl")
if(NOT EXISTS "${dart_file}")
  set(RunCMake_TEST_FAILED "DartConfiguration.tcl not found in build dir")
  return()
endif()
file(READ "${dart_file}" dart_content)
if(NOT dart_content MATCHES "BuildName: cli-build-name")
  set(RunCMake_TEST_FAILED
      "DartConfiguration.tcl does not contain BuildName: cli-build-name")
elseif(NOT dart_content MATCHES "Site: cli-site")
  set(RunCMake_TEST_FAILED
      "DartConfiguration.tcl does not contain Site: cli-site")
endif()
