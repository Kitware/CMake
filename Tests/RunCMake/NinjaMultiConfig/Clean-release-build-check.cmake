check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${TARGET_FILE_mylib_Release}
    ${TARGET_LINKER_FILE_mylib_Release}
    ${TARGET_SONAME_FILE_mylib_Release}
    ${TARGET_OBJECT_FILES_mylib_Release}

    ${TARGET_OBJECT_FILES_myobj_Release}

    ${TARGET_FILE_exeall_Release}
    ${TARGET_EXE_FILE_exeall_Release}
    ${TARGET_OBJECT_FILES_exeall_Release}

  EXCLUDE
    ${TARGET_OBJECT_FILES_exenotall_Release}
  )
