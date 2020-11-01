check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_FILE_release_only_tool_Release}
    ${TARGET_EXE_FILE_release_only_tool_Release}

  EXCLUDE
    ${TARGET_FILE_release_only_tool_Debug}
    ${TARGET_EXE_FILE_release_only_tool_Debug}
  )
