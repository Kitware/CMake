check_files("${RunCMake_TEST_BINARY_DIR}"
  EXCLUDE
    ${TARGET_OBJECT_FILES_exeall_Release}
    ${TARGET_OBJECT_FILES_exenotall_Release}
    ${TARGET_OBJECT_FILES_mylib_Release}
    ${TARGET_OBJECT_FILES_myobj_Release}
  )
