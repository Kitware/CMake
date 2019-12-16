check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_DEPENDS_SubdirCommand}
  )
check_file_contents("${TARGET_DEPENDS_SubdirCommand}" "^Genex config: Release\nINTDIR config: Release\n$")
