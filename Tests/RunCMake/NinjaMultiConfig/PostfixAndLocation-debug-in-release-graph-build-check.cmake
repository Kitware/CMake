check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_FILE_mylib_Release}
    ${TARGET_LINKER_FILE_mylib_Release}
    ${TARGET_FILE_mylib_Debug}
    ${TARGET_LINKER_FILE_mylib_Debug}
  )
