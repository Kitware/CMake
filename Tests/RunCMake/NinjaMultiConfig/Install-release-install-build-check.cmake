check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_FILE_exe_Release}
    ${TARGET_OBJECT_FILES_exe_Release}

    ${TARGET_FILE_mylib_Release}
    ${TARGET_LINKER_FILE_mylib_Release}
    ${TARGET_OBJECT_FILES_mylib_Release}

    ${RunCMake_TEST_BINARY_DIR}/install/bin/Release/${TARGET_FILE_NAME_exe_Release}
    ${RunCMake_TEST_BINARY_DIR}/install/lib/Release/${TARGET_FILE_NAME_mylib_Release}
    ${RunCMake_TEST_BINARY_DIR}/install/lib/Release/${TARGET_LINKER_FILE_NAME_mylib_Release}

  EXCLUDE
    ${TARGET_OBJECT_FILES_exe_Debug}
    ${TARGET_OBJECT_FILES_mylib_Debug}

    ${TARGET_OBJECT_FILES_exe_MinSizeRel}
    ${TARGET_OBJECT_FILES_mylib_MinSizeRel}

    ${TARGET_OBJECT_FILES_exe_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_mylib_RelWithDebInfo}
  )
