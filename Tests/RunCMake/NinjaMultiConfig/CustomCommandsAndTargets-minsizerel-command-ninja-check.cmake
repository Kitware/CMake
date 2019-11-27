check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_DEPENDS_SubdirCommand}
  )
check_file_contents("${TARGET_DEPENDS_SubdirCommand}" "^Genex config: MinSizeRel\nINTDIR config: MinSizeRel\n$")
