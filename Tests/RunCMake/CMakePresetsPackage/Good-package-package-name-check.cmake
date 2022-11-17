include("${RunCMake_TEST_BINARY_DIR}/default/CPackConfig.cmake")

file(READ "${RunCMake_TEST_BINARY_DIR}/default/${CPACK_PACKAGE_FILE_NAME}.json" contents)
string(JSON package_name GET "${contents}" packageName)
if(NOT package_name STREQUAL "package-name")
  set(RunCMake_TEST_FAILED "Expected package name to be \"package-name\" but it was \"${package_name}\"")
endif()
