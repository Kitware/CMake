include("${RunCMake_TEST_BINARY_DIR}/default/CPackConfig.cmake")

set(filename "${RunCMake_TEST_BINARY_DIR}/default/package-directory/_CPack_Packages/${CPACK_TOPLEVEL_TAG}/TGZ/${CPACK_PACKAGE_FILE_NAME}.tar.gz")
if(NOT EXISTS "${filename}")
  set(RunCMake_TEST_FAILED "Expected ${filename} to exist but it does not")
endif()
