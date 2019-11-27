check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_DEPENDS_SubdirCommand}
    ${TARGET_DEPENDS_TopCommand}
    ${TARGET_BYPRODUCTS_SubdirTarget}
    ${TARGET_BYPRODUCTS_TopTarget}
    ${TARGET_FILE_SubdirPostBuild_Debug}
  )
