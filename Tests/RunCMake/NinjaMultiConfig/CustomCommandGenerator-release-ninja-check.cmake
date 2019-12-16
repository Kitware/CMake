check_files("${RunCMake_TEST_BINARY_DIR}"
  INCLUDE
    ${CONFIG_FILES}
    ${GENERATED_FILES}

    ${TARGET_FILE_generator_Debug}
    ${TARGET_OBJECT_FILES_generator_Debug}

    ${TARGET_FILE_generated_Debug}
    ${TARGET_OBJECT_FILES_generated_Debug}

    ${TARGET_FILE_generatorlib_Debug}
    ${TARGET_LINKER_FILE_generatorlib_Debug}
    ${TARGET_OBJECT_FILES_generatorlib_Debug}

    ${TARGET_OBJECT_FILES_generatorobj_Debug}

    ${TARGET_OBJECT_FILES_emptyobj_Debug}

    ${TARGET_FILE_generator_Release}
    ${TARGET_OBJECT_FILES_generator_Release}

    ${TARGET_FILE_generated_Release}
    ${TARGET_OBJECT_FILES_generated_Release}

    ${TARGET_FILE_generatorlib_Release}
    ${TARGET_LINKER_FILE_generatorlib_Release}
    ${TARGET_OBJECT_FILES_generatorlib_Release}

    ${TARGET_OBJECT_FILES_generatorobj_Release}

    ${TARGET_OBJECT_FILES_emptyobj_Release}

  EXCLUDE
    ${TARGET_OBJECT_FILES_generator_MinSizeRel}
    ${TARGET_OBJECT_FILES_generated_MinSizeRel}
    ${TARGET_OBJECT_FILES_generatorlib_MinSizeRel}
    ${TARGET_OBJECT_FILES_generatorobj_MinSizeRel}
    ${TARGET_OBJECT_FILES_emptyobj_MinSizeRel}

    ${TARGET_OBJECT_FILES_generator_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_generated_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_generatorlib_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_generatorobj_RelWithDebInfo}
    ${TARGET_OBJECT_FILES_emptyobj_RelWithDebInfo}
  )
