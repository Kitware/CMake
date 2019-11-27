check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${RunCMake_TEST_BINARY_DIR}/global.txt
    ${RunCMake_TEST_BINARY_DIR}/Debug.txt
    ${RunCMake_TEST_BINARY_DIR}/Release.txt
    ${RunCMake_TEST_BINARY_DIR}/MinSizeRel.txt
    ${RunCMake_TEST_BINARY_DIR}/RelWithDebInfo.txt
  )
