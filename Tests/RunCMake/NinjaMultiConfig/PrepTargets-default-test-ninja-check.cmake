check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_FILE_testexe_Debug}
    ${TARGET_OBJECT_FILES_testexe_Debug}

    ${TARGET_FILE_testexe_Release}
    ${TARGET_OBJECT_FILES_testexe_Release}

  EXCLUDE
    ${TARGET_OBJECT_FILES_testexe_RelWithDebInfo}

    ${TARGET_OBJECT_FILES_installlib_Debug}
    ${TARGET_OBJECT_FILES_installlib_Release}
    ${TARGET_OBJECT_FILES_installlib_RelWithDebInfo}
  )
