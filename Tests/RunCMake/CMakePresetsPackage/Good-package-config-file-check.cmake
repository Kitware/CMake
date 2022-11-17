include("${RunCMake_TEST_BINARY_DIR}/default/CPackConfig.cmake")

set(filename "${RunCMake_TEST_BINARY_DIR}/default/_CPack_Packages/${CPACK_TOPLEVEL_TAG}/TGZ/config-file-alt.tar.gz")
if(NOT EXISTS "${filename}")
  set(RunCMake_TEST_FAILED "Expected ${filename} to exist but it does not")
endif()
