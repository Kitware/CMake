include("${RunCMake_TEST_BINARY_DIR}/default/CPackConfig.cmake")

file(READ "${RunCMake_TEST_BINARY_DIR}/default/${CPACK_PACKAGE_FILE_NAME}.json" contents)
string(JSON package_version GET "${contents}" packageVersion)
if(NOT package_version STREQUAL "1.0")
  set(RunCMake_TEST_FAILED "Expected package version to be \"1.0\" but it was \"${package_version}\"")
endif()
