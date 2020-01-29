check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${GENERATED_FILES}

    ${TARGET_FILE_simpleexe_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_simpleexe_RelWithDebInfo}

    ${TARGET_FILE_simpleshared_RelWithDebInfo}
    ${TARGET_LINKER_FILE_simpleshared_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_simpleshared_RelWithDebInfo}

    ${TARGET_FILE_simplestatic_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_simplestatic_RelWithDebInfo}

    ${TARGET_OBJECT_FILES_simpleobj_RelWithDebInfo}

  EXCLUDE
    ${TARGET_OBJECT_FILES_simpleexe_Debug}
    ${TARGET_OBJECT_FILES_simpleshared_Debug}
    ${TARGET_OBJECT_FILES_simplestatic_Debug}
    ${TARGET_OBJECT_FILES_simpleobj_Debug}

    ${TARGET_OBJECT_FILES_simpleexe_Release}
    ${TARGET_OBJECT_FILES_simpleshared_Release}
    ${TARGET_OBJECT_FILES_simplestatic_Release}
    ${TARGET_OBJECT_FILES_simpleobj_Release}

    ${TARGET_OBJECT_FILES_simpleexe_MinSizeRel}
    ${TARGET_OBJECT_FILES_simpleshared_MinSizeRel}
    ${TARGET_OBJECT_FILES_simplestatic_MinSizeRel}
    ${TARGET_OBJECT_FILES_simpleobj_MinSizeRel}
  )
