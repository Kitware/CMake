check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_DEPENDS_SubdirCommand}
    ${TARGET_DEPENDS_TopCommand}
    ${TARGET_BYPRODUCTS_SubdirTarget}
    ${TARGET_BYPRODUCTS_TopTarget}
    ${TARGET_FILE_SubdirPostBuild_Debug}
    ${TARGET_FILE_SubdirPostBuild_Release}
    ${TARGET_BYPRODUCTS_SubdirPostBuild}
  )
check_file_contents("${TARGET_BYPRODUCTS_SubdirPostBuild}" "^Genex config: Release\nINTDIR config: Release\n$")
