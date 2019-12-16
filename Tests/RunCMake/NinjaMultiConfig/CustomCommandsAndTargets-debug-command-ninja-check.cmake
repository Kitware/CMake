check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_DEPENDS_SubdirCommand}
    ${TARGET_DEPENDS_TopCommand}
  )
check_file_contents("${TARGET_DEPENDS_TopCommand}" "^Genex config: Debug\nINTDIR config: Debug\n$")
