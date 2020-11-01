check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_DEPENDS_SubdirCommand}
    ${TARGET_DEPENDS_TopCommand}
    ${TARGET_BYPRODUCTS_SubdirTarget}
    ${TARGET_BYPRODUCTS_TopTarget}
  )
check_file_contents("${TARGET_BYPRODUCTS_TopTarget}" "^Genex config: Debug\nINTDIR config: Debug\n$")
